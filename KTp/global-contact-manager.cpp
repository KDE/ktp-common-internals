/*
 * This file is part of telepathy-common-internals
 *
 * Copyright (C) 2012 David Edmundson <kde@davidedmundson.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "global-contact-manager.h"

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/ContactManager>

#include <KDebug>


namespace KTp {
class GlobalContactManagerPrivate {
public:
    Tp::AccountManagerPtr accountManager;
};
}

using namespace KTp;

GlobalContactManager::GlobalContactManager(const Tp::AccountManagerPtr &accountManager, QObject *parent) :
    QObject(parent),
    d(new GlobalContactManagerPrivate())
{
    d->accountManager = accountManager;

    Q_FOREACH(const Tp::AccountPtr &account, accountManager->allAccounts()) {
        onNewAccount(account);
    }
    connect(accountManager.data(), SIGNAL(newAccount(Tp::AccountPtr)), SLOT(onNewAccount(Tp::AccountPtr)));
}

GlobalContactManager::~GlobalContactManager()
{
    delete d;
}

Tp::Contacts GlobalContactManager::allKnownContacts() const
{
    Tp::Contacts allContacts;
    if (d->accountManager.isNull()) {
        return allContacts;
    }

    Q_FOREACH(const Tp::AccountPtr &account, d->accountManager->allAccounts()) {
        if (!account->connection().isNull() && account->connection()->contactManager()->state() == Tp::ContactListStateSuccess) {
            allContacts.unite(account->connection()->contactManager()->allKnownContacts());
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

    //contact manager connected, inform everyone of potential new contacts
    Q_EMIT allKnownContactsChanged(contactManager->allKnownContacts(), Tp::Contacts());

    connect(contactManager.data(), SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)), SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts)));
}

Tp::AccountPtr GlobalContactManager::accountForContact(const Tp::ContactPtr &contact) const
{
    return accountForConnection(contact->manager()->connection());
}

Tp::AccountPtr GlobalContactManager::accountForConnection(const Tp::ConnectionPtr &connection) const
{
    //loop through all accounts looking for a matching connection.
    //arguably inneficient, but no. of accounts is normally very low, and it's not called very often.
    Q_FOREACH(const Tp::AccountPtr &account, d->accountManager->allAccounts()) {
        if (account->connection() == connection) {
            return account;
        }
    }

    return Tp::AccountPtr();
}
