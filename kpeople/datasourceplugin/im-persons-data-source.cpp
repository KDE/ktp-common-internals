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
#include "debug.h"

#include <KPeopleBackend/AllContactsMonitor>

#include <KPluginFactory>
#include <KPluginLoader>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QPixmap>

using namespace KPeople;

class KTpAllContacts : public AllContactsMonitor
{
    Q_OBJECT
public:
    KTpAllContacts();
    ~KTpAllContacts();
    virtual QMap<QString, AbstractContact::Ptr> contacts() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void loadCache();
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onContactChanged();
    void onContactInvalidated();
    void onAllKnownContactsChanged(const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved);

private:
    QString createUri(const KTp::ContactPtr &contact) const;

    //presence names indexed by ConnectionPresenceType
    QHash<QString, KTp::ContactPtr> m_contacts;
    QMap<QString, AbstractContact::Ptr> m_contactVCards;
};

static const QString S_KPEOPLE_PROPERTY_ACCOUNT_PATH = QString::fromLatin1("telepathy-accountPath");
static const QString S_KPEOPLE_PROPERTY_CONTACT_ID = QString::fromLatin1("telepathy-contactId");
static const QString S_KPEOPLE_PROPERTY_PRESENCE = QString::fromLatin1("telepathy-presence");
const QHash<Tp::ConnectionPresenceType, QString> s_presenceStrings = {
        { Tp::ConnectionPresenceTypeUnset, QString() },
        { Tp::ConnectionPresenceTypeOffline, QString::fromLatin1("offline") },
        { Tp::ConnectionPresenceTypeAvailable, QString::fromLatin1("available") },
        { Tp::ConnectionPresenceTypeAway, QString::fromLatin1("away") },
        { Tp::ConnectionPresenceTypeExtendedAway, QString::fromLatin1("xa") },
        { Tp::ConnectionPresenceTypeHidden, QString::fromLatin1("hidden") }, //of 'offline' ?
        { Tp::ConnectionPresenceTypeBusy, QString::fromLatin1("busy") },
        { Tp::ConnectionPresenceTypeUnknown, QString() },
        { Tp::ConnectionPresenceTypeError, QString() }
};

class TelepathyContact : public KPeople::AbstractContact
{
public:
    virtual QVariant customProperty(const QString &key) const Q_DECL_OVERRIDE
    {
        if (m_contact && m_account) {
            if (key == AbstractContact::NameProperty)
                return m_contact->alias();
            else if(key == AbstractContact::GroupsProperty)
                return m_contact->groups();
            else if(key == S_KPEOPLE_PROPERTY_CONTACT_ID)
                return m_contact->id();
            else if(key == S_KPEOPLE_PROPERTY_ACCOUNT_PATH)
                return m_account->objectPath();
            else if(key == S_KPEOPLE_PROPERTY_PRESENCE)
                return s_presenceStrings.value(m_contact->presence().type());
            else if (key == AbstractContact::PictureProperty)
                return m_contact->avatarPixmap();
        }
        return m_properties[key];
    }

    void insertProperty(const QString &key, const QVariant &value)
    {
        m_properties[key] = value;
    }

    void setContact(const KTp::ContactPtr &contact)
    {
        m_contact = contact;
    }

    void setAccount(const Tp::AccountPtr &account)
    {
        m_account = account;
    }

private:
    KTp::ContactPtr m_contact;
    Tp::AccountPtr m_account;
    QVariantMap m_properties;
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
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/ktp");
    QDir().mkpath(path);
    db.setDatabaseName(path+QStringLiteral("/cache.db"));
    if (!db.open()) {
        qWarning() << "couldn't open database" << db.databaseName();
    }

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
        QExplicitlySharedDataPointer<TelepathyContact> addressee(new TelepathyContact);

        const QString accountId =  query.value(0).toString();
        const QString contactId =  query.value(1).toString();
        addressee->insertProperty(AbstractContact::NameProperty, query.value(2).toString());
        addressee->insertProperty(AbstractContact::PictureProperty, QUrl::fromLocalFile(query.value(3).toString()));

        if (!groupsList.isEmpty()) {
            QVariantList contactGroups;

            Q_FOREACH (const QString &groupIdStr, query.value(4).toString().split(QLatin1String(","))) {
                bool convSuccess;
                int groupId = groupIdStr.toInt(&convSuccess);
                if ((!convSuccess) || (groupId >= groupsList.count()))
                    continue;

                contactGroups.append(groupsList.at(groupId));
            }

            addressee->insertProperty(QStringLiteral("groups"), contactGroups);
        }

        addressee->insertProperty(S_KPEOPLE_PROPERTY_CONTACT_ID, contactId);
        addressee->insertProperty(S_KPEOPLE_PROPERTY_ACCOUNT_PATH, TP_QT_ACCOUNT_OBJECT_PATH_BASE + QLatin1Char('/') + accountId);
        addressee->insertProperty(S_KPEOPLE_PROPERTY_PRESENCE, s_presenceStrings[Tp::ConnectionPresenceTypeOffline]);

        const QString uri = QLatin1String("ktp://") + accountId + QLatin1Char('?') + contactId;

        m_contactVCards[uri] = addressee;
        Q_EMIT contactAdded(uri, addressee);
    }

    //now start fetching the up-to-date information
    connect(KTp::accountManager()->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
        this, SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    emitInitialFetchComplete(true);
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
        qCWarning(KTP_KPEOPLE) << "Failed to initialize AccountManager:" << op->errorName();
        qCWarning(KTP_KPEOPLE) << op->errorMessage();

        return;
    }

    qCDebug(KTP_KPEOPLE) << "Account manager ready";

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

        AbstractContact::Ptr vcard = m_contactVCards.value(uri);
        bool added = false;
        if (!vcard) {
            vcard = AbstractContact::Ptr(new TelepathyContact);
            m_contactVCards[uri] = vcard;
            added = true;
        }
        static_cast<TelepathyContact*>(vcard.data())->setContact(ktpContact);
        static_cast<TelepathyContact*>(vcard.data())->setAccount(KTp::contactManager()->accountForContact(ktpContact));

        m_contacts.insert(uri, ktpContact);

        if (added) {
            Q_EMIT contactAdded(uri, vcard);
        } else {
            Q_EMIT contactChanged(uri, vcard);
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
    const QString uri = createUri(contact);
    Q_EMIT contactChanged(uri, m_contactVCards.value(uri));
}

void KTpAllContacts::onContactInvalidated()
{
    const KTp::ContactPtr contact(qobject_cast<KTp::Contact*>(sender()));
    const QString uri = createUri(contact);

    m_contacts.remove(uri);

    //set to offline and emit changed
    AbstractContact::Ptr vcard = m_contactVCards[uri];
    TelepathyContact *tpContact = static_cast<TelepathyContact*>(vcard.data());
    tpContact->insertProperty(S_KPEOPLE_PROPERTY_PRESENCE, QStringLiteral("offline"));
    Q_EMIT contactChanged(uri, vcard);
}

QMap<QString, AbstractContact::Ptr> KTpAllContacts::contacts()
{
    return m_contactVCards;
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
    return QStringLiteral("ktp");
}

AllContactsMonitor* IMPersonsDataSource::createAllContactsMonitor()
{
    return new KTpAllContacts();
}

K_PLUGIN_FACTORY( IMPersonsDataSourceFactory, registerPlugin<IMPersonsDataSource>(); )
K_EXPORT_PLUGIN( IMPersonsDataSourceFactory("im_persons_data_source_plugin") )


#include "im-persons-data-source.moc"
