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

    QString protocolName = m_account->serviceName().isEmpty() ? m_account->protocolName() : m_account->serviceName();
    // Emit a signal to notify the storage that a new account has been constructed
    // FIXME: Some IM Accounts don't have an ID as such, e.g. Link-Local-XMPP.
    emit created(m_account->objectPath(),
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
    emit accountDestroyed(m_account->objectPath());
}

void Account::onConnectionChanged(const Tp::ConnectionPtr &connection)
{
    if (! connection.isNull()) {

        m_connection = connection;

        // We need to destroy ourselves if the connection goes down.
        connect(m_connection.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(deleteLater()));

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

        m_contacts.append(contact);

        // Become ready asynchronously with the avatar data, since this may take some time and we
        // don't want to delay the rest of the contact's attributes being ready.
        // Fire and forget because we can't do anything even if this fails.
        contact->manager()->upgradeContacts(QList<Tp::ContactPtr>() << contact, 
                                            Tp::Features() << Tp::Contact::FeatureAvatarData
                                                           << Tp::Contact::FeatureAvatarToken);

        // Connect to all its signals
        connect(contact.data(),
                SIGNAL(aliasChanged(QString)),
                SLOT(onContactAliasChanged(QString)));
        connect(contact.data(),
                SIGNAL(addedToGroup(QString)),
                SLOT(onContactAddedToGroup(QString)));
        connect(contact.data(),
                SIGNAL(removedFromGroup(QString)),
                SLOT(onContactRemovedFromGroup(QString)));
        connect(contact.data(),
                SIGNAL(avatarDataChanged(Tp::AvatarData)),
                SLOT(onContactAvatarChanged(Tp::AvatarData)));

        emit contactCreated(m_account->objectPath(), contact->id());
    }
}

void Account::onContactAddedToGroup(const QString& group)
{
    const Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));
    Q_ASSERT(contact);

    emit contactGroupsChanged(m_account->objectPath(), contact->id(), contact->groups());
}

void Account::onContactRemovedFromGroup(const QString& group)
{
    const Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));
    Q_ASSERT(contact);

    emit contactGroupsChanged(m_account->objectPath(), contact->id(), contact->groups());
}

void Account::onContactAliasChanged(const QString &alias)
{
    const Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));
    Q_ASSERT(contact);

    emit contactAliasChanged(m_account->objectPath(), contact->id(), alias);
}

void Account::onContactAvatarChanged(const Tp::AvatarData &avatar)
{
    const Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));
    Q_ASSERT(contact);

    emit contactAvatarChanged(m_account->objectPath(), contact->id(), avatar);
}

#include "account.moc"

