#include "global-contact-manager.h"

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/ContactManager>

#include <KDebug>

using namespace KTp;

class GlobalContactManagerPrivate {
public:
    Tp::AccountManagerPtr accountManager;
};

AccountContact::AccountContact(const Tp::AccountPtr &account, const Tp::ContactPtr &contact)
{
    m_account = account;
    m_contact = contact;
}

Tp::AccountPtr AccountContact::account() const
{
    return m_account;
}

Tp::ContactPtr AccountContact::contact() const
{
    return m_contact;
}


GlobalContactManager::GlobalContactManager(QObject *parent) :
    QObject(parent),
    d(new GlobalContactManagerPrivate())
{
}

GlobalContactManager::~GlobalContactManager()
{
    delete d;
}


void GlobalContactManager::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    if (! d->accountManager.isNull()) {
        kWarning() << "account manager already set";
        return;
    }

    if (!accountManager->isReady()) {
        kWarning() << "account manager not ready";
        return;
    }

    d->accountManager = accountManager;

    Q_FOREACH(const Tp::AccountPtr &account, accountManager->allAccounts()) {
        onNewAccount(account);
    }
    connect(accountManager.data(), SIGNAL(newAccount(Tp::AccountPtr)), SLOT(onNewAccount(Tp::AccountPtr)));
}

AccountContactList GlobalContactManager::allKnownContacts()
{
    AccountContactList allContacts;

    if (d->accountManager.isNull()) {
        return allContacts;
    }

    Q_FOREACH(const Tp::AccountPtr &account, d->accountManager->allAccounts()) {
        if (!account->connection().isNull() && account->connection()->contactManager()->state() == Tp::ContactListStateSuccess) {
            Q_FOREACH(const Tp::ContactPtr &contact, account->connection()->contactManager()->allKnownContacts()) {
                allContacts.append(AccountContact(account, contact));
            }
        }
    }
    return allContacts;
}

void GlobalContactManager::onNewAccount(const Tp::AccountPtr &account)
{
    onConnectionChanged(account->connection());
    connect(account.data(), SIGNAL(connectionChanged(Tp::ConnectionPtr)), SLOT(onConnectionChanged(Tp::ConnectionPtr)));
}


void GlobalContactManager::onConnectionChanged(const Tp::ConnectionPtr &connection)
{
    if (connection.isNull()) {
        return;
    }
    onContactManagerStateChanged(connection->contactManager(), connection->contactManager()->state());
    connect(connection->contactManager().data(), SIGNAL(stateChanged(Tp::ContactListState)), SLOT(onContactManagerStateChanged(Tp::ContactListState)));
}

void GlobalContactManager::onContactManagerStateChanged(Tp::ContactListState state)
{
    Tp::ContactManager* contactManager = qobject_cast<Tp::ContactManager*>(sender());
    Q_ASSERT(contactManager);
    onContactManagerStateChanged(Tp::ContactManagerPtr(contactManager), state);
}

void GlobalContactManager::onContactManagerStateChanged(const Tp::ContactManagerPtr &contactManager, Tp::ContactListState state)
{
    //contact manager still isn't ready. Do nothing.
    if (state != Tp::ContactListStateSuccess) {
        return;
    }

    onAllKnownContactsChanged(contactManager, contactManager->allKnownContacts(), Tp::Contacts(), Tp::Channel::GroupMemberChangeDetails());
    connect(contactManager.data(), SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)), SLOT(onAllKnownContactsChanged(Tp::Contacts,Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)));
}

void GlobalContactManager::onAllKnownContactsChanged(const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved, const Tp::Channel::GroupMemberChangeDetails &details)
{
    Tp::ContactManager* contactManager = qobject_cast<Tp::ContactManager*>(sender());
    Q_ASSERT(contactManager);
    onAllKnownContactsChanged(Tp::ContactManagerPtr(contactManager), contactsAdded, contactsRemoved, details);
}

void GlobalContactManager::onAllKnownContactsChanged(const Tp::ContactManagerPtr &contactManager, const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved, const Tp::Channel::GroupMemberChangeDetails &details)
{
    Q_UNUSED(details);
    const Tp::AccountPtr account = accountForContactManager(contactManager);

    AccountContactList contactsToAdd;
    AccountContactList contactsToRemove;

    Q_FOREACH (const Tp::ContactPtr &contact, contactsAdded) {
        contactsToAdd << AccountContact(account, contact);
    }
    Q_FOREACH (const Tp::ContactPtr &contact, contactsRemoved) {
        contactsToRemove << AccountContact(account, contact);
    }

    Q_EMIT allKnownContactsChanged(contactsToAdd, contactsToRemove);
}

Tp::AccountPtr GlobalContactManager::accountForContactManager(const Tp::ContactManagerPtr &contactManager) const
{
    Tp::ConnectionPtr connection = contactManager->connection();

    //loop through all accounts looking for a matching connection.
    //arguably inneficient, but no. of accounts is normally very low, and it's not called very often.
    Q_FOREACH(const Tp::AccountPtr &account, d->accountManager->allAccounts()) {
        if (account->connection() == connection) {
            return account;
        }
    }

    return Tp::AccountPtr();
}
