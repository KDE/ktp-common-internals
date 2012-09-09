#ifndef GLOBALCONTACTMANAGER_H
#define GLOBALCONTACTMANAGER_H

#include <QObject>

#include <TelepathyQt/Types>
#include <TelepathyQt/ContactManager>

#include <KTp/ktp-export.h>

namespace KTp {

class KTP_EXPORT AccountContact {
public:
    AccountContact(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);
    Tp::AccountPtr account() const;
    Tp::ContactPtr contact() const;
private:
    Tp::AccountPtr m_account;
    Tp::ContactPtr m_contact;
};

typedef QList<AccountContact> AccountContactList;

class GlobalContactManagerPrivate;

class KTP_EXPORT GlobalContactManager : public QObject
{
    Q_OBJECT
public:
    explicit GlobalContactManager(QObject *parent = 0);
    virtual ~GlobalContactManager();
    
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    AccountContactList allKnownContacts();

Q_SIGNALS:
    void allKnownContactsChanged(const AccountContactList &contactsAdded, const AccountContactList &contactsRemoved);
    void presencePublicationRequested(const AccountContactList &contacts);

private Q_SLOTS:
    void onNewAccount(const Tp::AccountPtr &account);
    void onConnectionChanged(const Tp::ConnectionPtr &connection);
    void onContactManagerStateChanged(Tp::ContactListState state);
    void onAllKnownContactsChanged (const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved, const Tp::Channel::GroupMemberChangeDetails &details);
    
private:
    void onContactManagerStateChanged(const Tp::ContactManagerPtr &contactManager, Tp::ContactListState state);
    void onAllKnownContactsChanged (const Tp::ContactManagerPtr &contactManager, const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved, const Tp::Channel::GroupMemberChangeDetails &details);

    Tp::AccountPtr accountForContactManager(const Tp::ContactManagerPtr &contactManager) const;

    GlobalContactManagerPrivate *d;
};
}

#endif // GLOBALCONTACTLIST_H
