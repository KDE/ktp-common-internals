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

#include "contact.h"

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
    connect(m_account.data(),
            SIGNAL(currentPresenceChanged(Tp::Presence)),
            SLOT(onCurrentPresenceChanged(Tp::Presence)));
    //    connect(m_account.data(),
    //            SIGNAL(avatarChanged(Tp::Avatar)),
    //            SLOT(onAvatarChanged(Tp::Avatar)));
    // ...... and any other properties we want to sync...
    connect(m_account.data(),
            SIGNAL(connectionChanged(Tp::ConnectionPtr)),
            SLOT(onConnectionChanged(Tp::ConnectionPtr)));

    QString protocolName = m_account->serviceName().isEmpty() ? m_account->protocolName() : m_account->serviceName();
    // Emit a signal to notify the storage that a new account has been constructed
    // FIXME: Some IM Accounts don't have an ID as such, e.g. Link-Local-XMPP.
    emit created(m_account->objectPath(),
                 m_account->parameters().value(QLatin1String("account")).toString(),
                 protocolName);

    // Simulate all the accounts properties being changed.
    onCurrentPresenceChanged(m_account->currentPresence());
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
    // Loop over all our children, and if they're Accounts, shut them down.
    foreach (Contact *contact, m_contacts.values()) {
        contact->shutdown();
    }

    kDebug();

    // Emit a signal to say we were destroyed.
    emit accountDestroyed(m_account->objectPath());
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

        // Create the hash containing all the contacts to notify the storage of the
        // full set of contacts that still exist when the account is connected.
        // This *must* be done before creating the contact wrapper objects.
        QList<QString> initialContacts;
        foreach (const Tp::ContactPtr &contact, contacts) {
            initialContacts.append(contact->id());
        }
        emit initialContactsLoaded(m_account->objectPath(), initialContacts);

        // Create wrapper objects for all the Contacts.
        foreach (const Tp::ContactPtr &contact, contacts) {
            onNewContact(contact);
        }
//        kDebug() << "Loop over.";
    }
}


void Account::onNicknameChanged(const QString& nickname)
{
    emit nicknameChanged(m_account->objectPath(), nickname);
}

void Account::onCurrentPresenceChanged(const Tp::Presence &presence)
{
    emit currentPresenceChanged(m_account->objectPath(), presence.barePresence());
}

void Account::onAllKnownContactsChanged(const Tp::Contacts& added, const Tp::Contacts& removed)
{
    // For each added contact, let's check if we already have a Contact wrapper for it
    foreach (const Tp::ContactPtr &contact, added) {
        if (!m_contacts.contains(contact)) {
            // It's a brand new one
            onNewContact(contact);
        }
    }

    // If contacts are removed, we don't actually need to do anything!
    Q_UNUSED(removed);
}

void Account::onNewContact(const Tp::ContactPtr &contact)
{
    // Only create a new contact if one doesn't already exist.
    if (!m_contacts.contains(contact)) {
        // Create a new Contact wrapper objectPath
        Contact *c = new Contact(contact, this);

        // Insert it into the Hash.
        m_contacts.insert(contact, c);

        // Connect to all its signals
        connect(c,
                SIGNAL(created(QString)),
                SLOT(onContactCreated(QString)));
        connect(c,
                SIGNAL(contactDestroyed(QString,Tp::ContactPtr)),
                SLOT(onContactDestroyed(QString,Tp::ContactPtr)));
        connect(c,
                SIGNAL(aliasChanged(QString,QString)),
                SLOT(onContactAliasChanged(QString,QString)));
        connect(c,
                SIGNAL(presenceChanged(QString,Tp::SimplePresence)),
                SLOT(onContactPresenceChanged(QString,Tp::SimplePresence)));
        connect(c,
                SIGNAL(groupsChanged(QString,QStringList)),
                SLOT(onContactGroupsChanged(QString,QStringList)));
        connect(c,
                SIGNAL(blockStatusChanged(QString,bool)),
                SLOT(onContactBlockStatusChanged(QString,bool)));
        connect(c,
                SIGNAL(publishStateChanged(QString,Tp::Contact::PresenceState)),
                SLOT(onContactPublishStateChanged(QString,Tp::Contact::PresenceState)));
        connect(c,
                SIGNAL(subscriptionStateChanged(QString,Tp::Contact::PresenceState)),
                SLOT(onContactSubscriptionStateChanged(QString,Tp::Contact::PresenceState)));
        connect(c,
                SIGNAL(capabilitiesChanged(QString,Tp::ConnectionPtr,Tp::ContactCapabilities)),
                SLOT(onContactCapabilitiesChanged(QString,Tp::ConnectionPtr,Tp::ContactCapabilities)));
        connect(c,
                SIGNAL(avatarChanged(QString,Tp::AvatarData)),
                SLOT(onContactAvatarChanged(QString,Tp::AvatarData)));

        c->init();
    }
}

void Account::onContactCreated(const QString &id)
{
    emit contactCreated(m_account->objectPath(), id);
}

void Account::onContactDestroyed(const QString &id, const Tp::ContactPtr &contact)
{
    m_contacts.remove(contact);

    // Relay this signal to the controller
    emit contactDestroyed(m_account->objectPath(), id);
}

void Account::onContactAliasChanged(const QString &id, const QString &alias)
{
    emit contactAliasChanged(m_account->objectPath(), id, alias);
}

void Account::onContactPresenceChanged(const QString &id, const Tp::SimplePresence &presence)
{
    emit contactPresenceChanged(m_account->objectPath(), id, presence);
}

void Account::onContactGroupsChanged(const QString &id, const QStringList &groups)
{
    emit contactGroupsChanged(m_account->objectPath(), id, groups);
}

void Account::onContactBlockStatusChanged(const QString &id, bool blocked)
{
    emit contactBlockStatusChanged(m_account->objectPath(), id, blocked);
}

void Account::onContactPublishStateChanged(const QString &id, const Tp::Contact::PresenceState &state)
{
    emit contactPublishStateChanged(m_account->objectPath(), id, state);
}

void Account::onContactSubscriptionStateChanged(const QString &id, const Tp::Contact::PresenceState &state)
{
    emit contactSubscriptionStateChanged(m_account->objectPath(), id, state);
}

void Account::onContactCapabilitiesChanged(const QString &id, const Tp::ConnectionPtr &connection, const Tp::ContactCapabilities &capabilities)
{
    emit contactCapabilitiesChanged(m_account->objectPath(), id, connection, capabilities);
}

void Account::onContactAvatarChanged(const QString &id, const Tp::AvatarData &avatar)
{
    emit contactAvatarChanged(m_account->objectPath(), id, avatar);
}



#include "account.moc"

