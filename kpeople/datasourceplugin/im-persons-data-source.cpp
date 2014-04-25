/*
    Copyright (C) 2013  Martin Klapetek <mklapetek@kde.org>
    Copyright (C) 2014  David Edmundson <davidedmundson@kde.org>


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "im-persons-data-source.h"

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Presence>

#include "KTp/contact-factory.h"
#include "KTp/global-contact-manager.h"
#include "KTp/types.h"

#include <KPeople/AllContactsMonitor>
#include <KDE/KABC/Addressee>

#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>
#include <KPluginFactory>
#include <KPluginLoader>

#include <QSqlDatabase>
#include <QSqlQuery>

using namespace KPeople;


class KTpAllContacts : public AllContactsMonitor
{
    Q_OBJECT
public:
    KTpAllContacts();
    ~KTpAllContacts();
    virtual KABC::Addressee::Map contacts();

private Q_SLOTS:
    void loadCache();
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onContactChanged();
    void onContactInvalidated();
    void onAllKnownContactsChanged(const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved);

private:
    QString createUri(const KTp::ContactPtr &contact) const;
    KABC::Addressee contactToAddressee(const Tp::ContactPtr &contact) const;
    QHash<QString, KTp::ContactPtr> m_contacts;
    KABC::Addressee::Map m_contactVCards;
};

KTpAllContacts::KTpAllContacts()
{
    Tp::registerTypes();

    loadCache();
}

KTpAllContacts::~KTpAllContacts()
{
}

void KTpAllContacts::loadCache()
{
    QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), QLatin1String("ktpCache"));
    db.setDatabaseName(KGlobal::dirs()->locateLocal("data", QLatin1String("ktp/cache.db")));
    db.open();

    QSqlQuery query(db);
    query.exec(QLatin1String("SELECT groupName FROM groups ORDER BY groupId;"));

    QStringList groupsList;
    while (query.next()) {
        groupsList.append(query.value(0).toString());
    }

    if (!groupsList.isEmpty()) {
        query.exec(QLatin1String("SELECT accountId, contactId, alias, avatarFileName, groupsIds FROM contacts;"));
    } else {
        query.exec(QLatin1String("SELECT accountId, contactId, alias, avatarFileName FROM contacts;"));
    }

    while (query.next()) {
        KABC::Addressee addressee;

        const QString accountId =  query.value(0).toString();
        const QString contactId =  query.value(1).toString();
        addressee.setFormattedName(query.value(2).toString());
        addressee.setPhoto(KABC::Picture(query.value(3).toString()));

        if (!groupsList.isEmpty()) {
            QStringList contactGroups;

            Q_FOREACH (const QString &groupIdStr, query.value(4).toString().split(QLatin1String(","))) {
                bool convSuccess;
                int groupId = groupIdStr.toInt(&convSuccess);
                if ((!convSuccess) || (groupId >= groupsList.count()))
                    continue;

                contactGroups.append(groupsList.at(groupId));
            }

            addressee.setCategories(contactGroups);
        }

        addressee.insertCustom(QLatin1String("telepathy"), QLatin1String("contactId"), contactId);
        addressee.insertCustom(QLatin1String("telepathy"), QLatin1String("accountPath"), accountId);
        addressee.insertCustom(QLatin1String("telepathy"), QLatin1String("presence"), QLatin1String("offline"));

        const QString uri = QLatin1String("ktp://") + accountId + QLatin1Char('?') + contactId;

        m_contactVCards[uri] = addressee;
        Q_EMIT contactAdded(uri, addressee);
    }

    //now start fetching the up-to-date information
    connect(KTp::accountManager()->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
        this, SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    emitInitialFetchComplete();
}

QString KTpAllContacts::createUri(const KTp::ContactPtr &contact) const
{
    // so real ID will look like
    // ktp://gabble/jabber/blah/asdfjwer?foo@bar.com
    // ? is used as it is not a valid character in the dbus path that makes up the account UID
    return QLatin1String("ktp://") + contact->accountUniqueIdentifier() + QLatin1Char('?') + contact->id();
}

void KTpAllContacts::onAccountManagerReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Failed to initialize AccountManager:" << op->errorName();
        kWarning() << op->errorMessage();

        return;
    }

    kDebug() << "Account manager ready";

    connect(KTp::contactManager(), SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts)),
            this, SLOT(onAllKnownContactsChanged(Tp::Contacts,Tp::Contacts)));

    onAllKnownContactsChanged(KTp::contactManager()->allKnownContacts(), Tp::Contacts());
}

void KTpAllContacts::onAllKnownContactsChanged(const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved)
{
    if (!m_contacts.isEmpty()) {
        Q_FOREACH (const Tp::ContactPtr &c, contactsRemoved) {
            const KTp::ContactPtr &contact = KTp::ContactPtr::qObjectCast(c);
            const QString uri = createUri(contact);
            m_contacts.remove(uri);
            m_contactVCards.remove(uri);
            Q_EMIT contactRemoved(uri);
        }
    }

    Q_FOREACH (const Tp::ContactPtr &contact, contactsAdded) {
        KTp::ContactPtr ktpContact = KTp::ContactPtr::qObjectCast(contact);
        const QString uri = createUri(ktpContact);

        const KABC::Addressee vcard = contactToAddressee(contact);

        m_contacts.insert(uri, ktpContact);

        //TODO OPTIMISATION if we already exist we shouldn't create a whole new KABC object, just update the existing one
        //onContactChanged should be split into the relevant onAliasChanged etc.
        if (m_contactVCards.contains(uri)) {
            m_contactVCards[uri] = vcard;
            Q_EMIT contactChanged(uri, vcard);
        } else {
            m_contactVCards.insert(uri, vcard);
            Q_EMIT contactAdded(uri, vcard);
        }

        connect(ktpContact.data(), SIGNAL(presenceChanged(Tp::Presence)),
                this, SLOT(onContactChanged()));

        connect(ktpContact.data(), SIGNAL(capabilitiesChanged(Tp::ContactCapabilities)),
                this, SLOT(onContactChanged()));

        connect(ktpContact.data(), SIGNAL(invalidated()),
                this, SLOT(onContactInvalidated()));

        connect(ktpContact.data(), SIGNAL(avatarDataChanged(Tp::AvatarData)),
                this, SLOT(onContactChanged()));

        connect(ktpContact.data(), SIGNAL(addedToGroup(QString)),
                this, SLOT(onContactChanged()));

        connect(ktpContact.data(), SIGNAL(removedFromGroup(QString)),
                this, SLOT(onContactChanged()));
    }
}

void KTpAllContacts::onContactChanged()
{
    const KTp::ContactPtr contact(qobject_cast<KTp::Contact*>(sender()));
    m_contactVCards.insert(createUri(contact), contactToAddressee(contact));
    Q_EMIT contactChanged(createUri(contact), contactToAddressee(contact));
}

void KTpAllContacts::onContactInvalidated()
{
    const KTp::ContactPtr contact(qobject_cast<KTp::Contact*>(sender()));
    const QString uri = createUri(contact);

    m_contacts.remove(uri);

    //set to offline and emit changed
    KABC::Addressee &vcard = m_contactVCards[uri];
    vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("presence"), QLatin1String("offline"));
    Q_EMIT contactChanged(uri, vcard);
}

KABC::Addressee::Map KTpAllContacts::contacts()
{
    return m_contactVCards;
}

KABC::Addressee KTpAllContacts::contactToAddressee(const Tp::ContactPtr &contact) const
{
    KABC::Addressee vcard;
    Tp::AccountPtr account = KTp::contactManager()->accountForContact(contact);
    if (contact && account) {
        vcard.setFormattedName(contact->alias());
        vcard.setCategories(contact->groups());
        vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("contactId"), contact->id());
        vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("accountPath"), account->objectPath());
        vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("presence"), contact->presence().status());
        if (!contact->avatarData().fileName.isEmpty()) {
            vcard.setPhoto(KABC::Picture(contact->avatarData().fileName));
        }
    }
    return vcard;
}

IMPersonsDataSource::IMPersonsDataSource(QObject *parent, const QVariantList &args)
    : BasePersonsDataSource(parent)
{
    Q_UNUSED(args);
}

IMPersonsDataSource::~IMPersonsDataSource()
{
}

QString IMPersonsDataSource::sourcePluginId() const
{
    return QLatin1String("ktp");
}

AllContactsMonitor* IMPersonsDataSource::createAllContactsMonitor()
{
    return new KTpAllContacts();
}

K_PLUGIN_FACTORY( IMPersonsDataSourceFactory, registerPlugin<IMPersonsDataSource>(); )
K_EXPORT_PLUGIN( IMPersonsDataSourceFactory("im_persons_data_source_plugin") )


#include "im-persons-data-source.moc"
