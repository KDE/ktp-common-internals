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
#include <TelepathyQt/Utils>

#include "KTp/contact-factory.h"
#include "KTp/global-contact-manager.h"
#include "KTp/types.h"
#include "debug.h"

#include <KPeopleBackend/AllContactsMonitor>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KConfig>
#include <KConfigGroup>

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
    void loadCache(const QString &accountId = QString());
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onContactChanged();
    void onContactInvalidated();
    void onAllKnownContactsChanged(const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved);
    void onAccountCurrentPresenceChanged(const Tp::Presence &currentPresence);

private:
    //presence names indexed by ConnectionPresenceType
    QMap<QString, AbstractContact::Ptr> m_contactVCards;
};

class TelepathyContact : public KPeople::AbstractContact
{
public:
    virtual QVariant customProperty(const QString &key) const Q_DECL_OVERRIDE
    {
        // Check if the contact is valid first
        if (m_contact && m_contact->manager() && m_contact->manager()->connection() && m_account) {
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

void KTpAllContacts::loadCache(const QString &accountId)
{
    QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), QLatin1String("ktpCache"));
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/ktp");
    QDir().mkpath(path);
    db.setDatabaseName(path+QStringLiteral("/cache.db"));
    if (!db.open()) {
        qWarning() << "couldn't open database" << db.databaseName();
    }

    QSqlQuery query(db);
    query.exec(QLatin1String("SELECT DISTINCT groupName FROM groups ORDER BY groupId;"));

    QStringList groupsList;
    while (query.next()) {
        groupsList.append(query.value(0).toString());
    }

    QString queryPrep = QStringLiteral("SELECT accountId, contactId, alias, avatarFileName, isBlocked");
    if (!groupsList.isEmpty()) {
        queryPrep.append(QStringLiteral(", groupsIds"));
    }
    queryPrep.append(QStringLiteral(" FROM contacts"));

    if (!accountId.isEmpty()) {
        queryPrep.append(QStringLiteral(" WHERE accountId = ?;"));
        query.prepare(queryPrep);
        query.bindValue(0, accountId);
    } else {
        queryPrep.append(QStringLiteral(";"));
        query.prepare(queryPrep);
    }

    query.exec();

    KConfig config(QLatin1String("ktelepathy-avatarsrc"));
    QString cacheDir = QString::fromLatin1(qgetenv("XDG_CACHE_HOME"));
    if (cacheDir.isEmpty()) {
        cacheDir = QStringLiteral("%1/.cache").arg(QLatin1String(qgetenv("HOME")));
    }

    while (query.next()) {

        const QString accountId =  query.value(0).toString();
        const QString contactId =  query.value(1).toString();
        QString avatarFileName = query.value(3).toString();

        const QString uri = QLatin1String("ktp://") + accountId + QLatin1Char('?') + contactId;
        QExplicitlySharedDataPointer<TelepathyContact> addressee;
        bool found = false;
        {
            QMap<QString, AbstractContact::Ptr>::const_iterator it = m_contactVCards.constFind(uri);
            found = it!=m_contactVCards.constEnd();
            if (found)
                addressee = QExplicitlySharedDataPointer<TelepathyContact>(static_cast<TelepathyContact*>(it->data()));
            else
                addressee = QExplicitlySharedDataPointer<TelepathyContact>(new TelepathyContact);
        }

        addressee->insertProperty(AbstractContact::NameProperty, query.value(2).toString());

        if (avatarFileName.isEmpty()) {
            KConfigGroup avatarTokenGroup = config.group(contactId);
            QString avatarToken = avatarTokenGroup.readEntry(QLatin1String("avatarToken"));
            //only bother loading the pixmap if the token is not empty
            if (!avatarToken.isEmpty()) {
                // the accountId is in form of "connection manager name / protocol / username...",
                // so let's look for the first / after the very first / (ie. second /)
                QString path = QStringLiteral("%1/telepathy/avatars/%2").
                    arg(cacheDir).arg(accountId.left(accountId.indexOf(QLatin1Char('/'), accountId.indexOf(QLatin1Char('/')) + 1)));

                avatarFileName = QStringLiteral("%1/%2").arg(path).arg(Tp::escapeAsIdentifier(avatarToken));
            }
        }

        addressee->insertProperty(AbstractContact::PictureProperty, QUrl::fromLocalFile(avatarFileName));
        addressee->insertProperty(S_KPEOPLE_PROPERTY_IS_BLOCKED, query.value(4).toBool());

        if (!groupsList.isEmpty()) {
            QVariantList contactGroups;

            Q_FOREACH (const QString &groupIdStr, query.value(5).toString().split(QLatin1Char('/'))) {
                bool convSuccess;
                int groupId = groupIdStr.toInt(&convSuccess);
                if ((!convSuccess) || (groupId >= groupsList.count()))
                    continue;

                contactGroups.append(groupsList.at(groupId));
            }

            addressee->insertProperty(AbstractContact::GroupsProperty, contactGroups);
        }

        addressee->insertProperty(S_KPEOPLE_PROPERTY_CONTACT_ID, contactId);
        addressee->insertProperty(S_KPEOPLE_PROPERTY_ACCOUNT_PATH, TP_QT_ACCOUNT_OBJECT_PATH_BASE + QLatin1Char('/') + accountId);
        addressee->insertProperty(S_KPEOPLE_PROPERTY_PRESENCE, s_presenceStrings[Tp::ConnectionPresenceTypeOffline]);

        addressee->insertProperty(S_KPEOPLE_PROPERTY_CONTACT_URI, uri);

        if (found) {
            Q_EMIT contactChanged(uri, addressee);
        } else {
            Q_EMIT contactAdded(uri, addressee);
        }

        m_contactVCards[uri] = addressee;
    }

    //now start fetching the up-to-date information
    connect(KTp::accountManager()->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
        this, SLOT(onAccountManagerReady(Tp::PendingOperation*)), Qt::UniqueConnection);

    emitInitialFetchComplete(true);
}

void KTpAllContacts::onAccountManagerReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qCWarning(KTP_KPEOPLE) << "Failed to initialize AccountManager:" << op->errorName();
        qCWarning(KTP_KPEOPLE) << op->errorMessage();

        return;
    }

    qCDebug(KTP_KPEOPLE) << "Account manager ready";

    Q_FOREACH (const Tp::AccountPtr &account, KTp::accountManager()->allAccounts()) {
        connect(account.data(), &Tp::Account::currentPresenceChanged, this, &KTpAllContacts::onAccountCurrentPresenceChanged);
    }

    connect(KTp::contactManager(), SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts)),
            this, SLOT(onAllKnownContactsChanged(Tp::Contacts,Tp::Contacts)));

    onAllKnownContactsChanged(KTp::contactManager()->allKnownContacts(), Tp::Contacts());
}

void KTpAllContacts::onAccountCurrentPresenceChanged(const Tp::Presence &currentPresence)
{
    Tp::Account *account = qobject_cast<Tp::Account*>(sender());
    if (!account) {
        return;
    }

    if (currentPresence.type() == Tp::ConnectionPresenceTypeOffline) {
        loadCache(account->uniqueIdentifier());
    }
}

void KTpAllContacts::onAllKnownContactsChanged(const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved)
{
    if (!m_contactVCards.isEmpty()) {
        Q_FOREACH (const Tp::ContactPtr &c, contactsRemoved) {
            const KTp::ContactPtr &contact = KTp::ContactPtr::qObjectCast(c);
            const QString uri = contact->uri();
            m_contactVCards.remove(uri);
            Q_EMIT contactRemoved(uri);
        }
    }

    Q_FOREACH (const Tp::ContactPtr &contact, contactsAdded) {
        KTp::ContactPtr ktpContact = KTp::ContactPtr::qObjectCast(contact);
        const QString uri = ktpContact->uri();

        AbstractContact::Ptr vcard = m_contactVCards.value(uri);
        bool added = false;
        if (!vcard) {
            QExplicitlySharedDataPointer<TelepathyContact> tpc(new TelepathyContact);
            vcard = AbstractContact::Ptr(tpc);
            tpc->insertProperty(S_KPEOPLE_PROPERTY_CONTACT_URI, uri);
            m_contactVCards[uri] = vcard;
            added = true;
        }
        static_cast<TelepathyContact*>(vcard.data())->setContact(ktpContact);
        static_cast<TelepathyContact*>(vcard.data())->setAccount(KTp::contactManager()->accountForContact(ktpContact));

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
    const QString uri = contact->uri();
    Q_EMIT contactChanged(uri, m_contactVCards.value(uri));
}

void KTpAllContacts::onContactInvalidated()
{
    const KTp::ContactPtr contact(qobject_cast<KTp::Contact*>(sender()));
    const QString uri = contact->uri();

    //set to offline and emit changed
    AbstractContact::Ptr vcard = m_contactVCards.value(uri);
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

K_PLUGIN_FACTORY_WITH_JSON( IMPersonsDataSourceFactory, "im_persons_data_source_plugin.json", registerPlugin<IMPersonsDataSource>(); )
K_EXPORT_PLUGIN( IMPersonsDataSourceFactory("im_persons_data_source_plugin") )


#include "im-persons-data-source.moc"
