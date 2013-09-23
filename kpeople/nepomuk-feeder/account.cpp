/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2009-2011 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
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

#include "account.h"

#include <KDebug>

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingReady>

Account::Account(const Tp::AccountPtr &account, QObject *parent)
 : QObject(parent),
   m_account(account)
{
    // Do nothing here. Wait for init() to be called to allow
    // signal/slot connections to be set up first.
    kDebug() << "Account Constructed.";
}

void Account::init()
{
    // Connect to all the signals that indicate changes in properties we care about.
    connect(m_account.data(),
            SIGNAL(nicknameChanged(QString)),
            SLOT(onNicknameChanged(QString)));
    //    connect(m_account.data(),
    //            SIGNAL(avatarChanged(Tp::Avatar)),
    //            SLOT(onAvatarChanged(Tp::Avatar)));
    // ...... and any other properties we want to sync...
    connect(m_account.data(),
            SIGNAL(connectionChanged(Tp::ConnectionPtr)),
            SLOT(onConnectionChanged(Tp::ConnectionPtr)));

    connect(m_account.data(),
            SIGNAL(removed()),
            SLOT(onAccountRemoved()));

    QString protocolName = m_account->serviceName().isEmpty() ? m_account->protocolName() : m_account->serviceName();
    // Emit a signal to notify the storage that a new account has been constructed
    // FIXME: Some IM Accounts don't have an ID as such, e.g. Link-Local-XMPP.
    Q_EMIT created(m_account->objectPath(),
                 m_account->parameters().value(QLatin1String("account")).toString(),
                 protocolName);

    // Simulate all the accounts properties being changed.
    onNicknameChanged(m_account->nickname());

    // Now that the storage stuff is done, simulate emission of all the account signals.
    onConnectionChanged(m_account->connection());
}

Account::~Account()
{
    kDebug();
}

void Account::shutdown()
{
    kDebug();

    // Emit a signal to say we were destroyed.
    Q_EMIT accountDestroyed(m_account->objectPath());
}

void Account::onConnectionChanged(const Tp::ConnectionPtr &connection)
{
    if (! connection.isNull()) {

        m_connection = connection;

        if (!m_connection->contactManager()) {
            kWarning() << "ContactManager is Null. Abort getting contacts.";
            return;
        }

        //add contacts as soon as the contact manager is ready.
        connect(m_connection->contactManager().data(),
                SIGNAL(stateChanged(Tp::ContactListState)),
                SLOT(onContactManagerStateChanged(Tp::ContactListState)));
        connect(m_connection->contactManager().data(),
                SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)),
                SLOT(onAllKnownContactsChanged(Tp::Contacts,Tp::Contacts)));
        // Simulate a state change signal in case it is already ready.
        onContactManagerStateChanged(m_connection->contactManager()->state());

    } else {
        // Connection has gone down. Delete our pointer to it.
        m_connection.reset();
        kDebug() << "Connection closed:" << this;
    }
}

void Account::onContactManagerStateChanged(Tp::ContactListState state)
{
//    kDebug() << "contact manager state changed to " << state;

    if (state == Tp::ContactListStateSuccess)  {
        Tp::Contacts contacts = m_connection->contactManager()->allKnownContacts();

        Q_EMIT initialContactsLoaded(m_account->objectPath(), contacts);

        // Create wrapper objects for all the Contacts.
        Q_FOREACH (const Tp::ContactPtr &contact, contacts) {
            onNewContact(contact);
        }
//        kDebug() << "Loop over.";
    }
}

void Account::onNicknameChanged(const QString &nickname)
{
    Q_EMIT nicknameChanged(m_account->objectPath(), nickname);
}

void Account::onAllKnownContactsChanged(const Tp::Contacts &added, const Tp::Contacts &removed)
{
    // For each added contact, let's check if we already have a Contact wrapper for it
    Q_FOREACH (const Tp::ContactPtr &contact, added) {
        if (!m_contacts.contains(contact)) {
            // It's a brand new one
            onNewContact(contact);
        }
    }

    Q_FOREACH (const Tp::ContactPtr &contact, removed) {
        onContactRemoved(contact);
    }
}

void Account::onAccountRemoved()
{
    Tp::AccountPtr account(qobject_cast<Tp::Account*>(sender()));
    Q_ASSERT(account);

    kDebug() << "Account being removed";

    Q_EMIT accountRemoved(account->objectPath());
}

void Account::onNewContact(const Tp::ContactPtr &contact)
{
    // Only create a new contact if one doesn't already exist.
    if (!m_contacts.contains(contact)) {

        m_contacts.append(contact);

        // Connect to all its signals
        connect(contact.data(),
                SIGNAL(aliasChanged(QString)),
                SLOT(onContactAliasChanged()));
        connect(contact.data(),
                SIGNAL(addedToGroup(QString)),
                SLOT(onContactAddedToGroup()));
        connect(contact.data(),
                SIGNAL(removedFromGroup(QString)),
                SLOT(onContactRemovedFromGroup(QString)));
        connect(contact.data(),
                SIGNAL(avatarDataChanged(Tp::AvatarData)),
                SLOT(onContactAvatarChanged(Tp::AvatarData)));
        Q_EMIT contactCreated(m_account->objectPath(), contact);
    }
}

void Account::onContactRemoved(const Tp::ContactPtr& contact)
{
    m_contacts.removeAll(contact);
    Q_EMIT contactRemoved(m_account->objectPath(), contact);
}

void Account::onContactAddedToGroup()
{
    const Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));
    Q_ASSERT(contact);

    onContactAddedToGroup(contact);
}

void Account::onContactAddedToGroup(const Tp::ContactPtr &contact)
{
    Q_EMIT contactGroupsChanged(m_account->objectPath(), contact->id(), contact->groups());
}

void Account::onContactRemovedFromGroup(const QString &group)
{
    Q_UNUSED(group);
    const Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));
    Q_ASSERT(contact);

    Q_EMIT contactGroupsChanged(m_account->objectPath(), contact->id(), contact->groups());
}

void Account::onContactAliasChanged()
{
    const Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));
    Q_ASSERT(contact);

    onContactAliasChanged(contact);
}

void Account::onContactAliasChanged(const Tp::ContactPtr &contact)
{
    Q_EMIT contactAliasChanged(m_account->objectPath(), contact->id(), contact->alias());
}

void Account::onContactAvatarChanged(const Tp::AvatarData &avatar)
{
    const Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));
    Q_ASSERT(contact);

    Q_EMIT contactAvatarChanged(m_account->objectPath(), contact->id(), avatar);
}

#include "account.moc"
