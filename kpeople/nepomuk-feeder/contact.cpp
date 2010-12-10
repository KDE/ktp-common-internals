/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
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

#include "contact.h"

#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactCapabilities>
#include <TelepathyQt4/ContactManager>

#include <KDebug>

Contact::Contact(const Tp::ContactPtr &contact, QObject *parent)
 : QObject(parent),
   m_contact(contact)
{
    // Do nothing here because the signals/slots need to be connected in our
    // parent class before we start doing stuff.
    kDebug() << "Creating new contact";
}

void Contact::init()
{
    // We need to destroy ourself if the connection goes down.
    connect(m_contact->manager()->connection().data(),
            SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(deleteLater()));

    // Connect to signals for all properties we want to keep synced.
    connect(m_contact.data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SLOT(onPresenceChanged(Tp::Presence)));
    connect(m_contact.data(),
            SIGNAL(aliasChanged(QString)),
            SLOT(onAliasChanged(QString)));
    connect(m_contact.data(),
            SIGNAL(addedToGroup(QString)),
            SLOT(onAddedToGroup(QString)));
    connect(m_contact.data(),
            SIGNAL(removedFromGroup(QString)),
            SLOT(onRemovedFromGroup(QString)));
    connect(m_contact.data(),
            SIGNAL(capabilitiesChanged(Tp::ContactCapabilities)),
            SLOT(onCapabilitiesChanged(Tp::ContactCapabilities)));
    connect(m_contact.data(),
            SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState, Tp::Channel::GroupMemberChangeDetails)),
            SLOT(onSubscriptionStateChanged(Tp::Contact::PresenceState)));
    connect(m_contact.data(),
            SIGNAL(publishStateChanged(Tp::Contact::PresenceState, Tp::Channel::GroupMemberChangeDetails)),
            SLOT(onPublishStateChanged(Tp::Contact::PresenceState)));
    connect(m_contact.data(),
            SIGNAL(blockStatusChanged(bool, Tp::Channel::GroupMemberChangeDetails)),
            SLOT(onBlockStatusChanged(bool)));
    // FIXME: Connect to any other signals of sync-worthy properties here.

    // Emit a signal to notify the controller that a new contact has been created.
    emit created(m_contact->id());

    // Synthesize all the properties being changed
    // FIXME: Make this bit correct
    /*
    onPublishStateChanged(m_contact->publishState());
    onSubscriptionStateChanged(m_contact->subscriptionState());
    onBlockStatusChanged(m_contact->isBlocked());
    if (contact->capabilities() != 0) {
        onCapabilitiesChanged(m_contact->capabilities());
    }
    */
}

Contact::~Contact()
{
    kDebug();
}

void Contact::shutdown()
{
    // Signal this contact is destroyed so it can be removed from the Hash.
    emit contactDestroyed(m_contact->id(), m_contact);
}


void Contact::onAliasChanged(const QString& alias)
{
    emit aliasChanged(m_contact->id(), alias);
}

void Contact::onPresenceChanged(const Tp::Presence &presence)
{
    emit presenceChanged(m_contact->id(), presence.barePresence());
}

void Contact::onAddedToGroup(const QString &group)
{
    kDebug() << "On added to group " << group;

    emit addedToGroup(m_contact->id(), group);
}

void Contact::onRemovedFromGroup(const QString &group)
{
    kDebug() << "On removed from group " << group;

    emit removedFromGroup(m_contact->id(), group);
}

void Contact::onBlockStatusChanged(bool blocked)
{
    emit blockStatusChanged(m_contact->id(), blocked);
}

void Contact::onPublishStateChanged(Tp::Contact::PresenceState state)
{
    emit publishStateChanged(m_contact->id(), state);
}

void Contact::onSubscriptionStateChanged(Tp::Contact::PresenceState state)
{
    emit subscriptionStateChanged(m_contact->id(), state);
}

void Contact::onCapabilitiesChanged(const Tp::ContactCapabilities &capabilities)
{
    // FIXME: Implement me!
}


#include "contact.moc"

