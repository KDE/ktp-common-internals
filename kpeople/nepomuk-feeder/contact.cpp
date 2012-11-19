/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2010-2011 Collabora Ltd. <info@collabora.co.uk>
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

#include <TelepathyQt/Connection>
#include <TelepathyQt/Contact>
#include <TelepathyQt/ContactCapabilities>
#include <TelepathyQt/ContactManager>

#include <KDebug>

Contact::Contact(const Tp::ContactPtr &contact, QObject *parent)
 : QObject(parent),
   m_contact(contact)
{
    // Do nothing here because the signals/slots need to be connected in our
    // parent class before we start doing stuff.
    //kDebug() << "Creating new contact";
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
            SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
            SLOT(onSubscriptionStateChanged(Tp::Contact::PresenceState)));
    connect(m_contact.data(),
            SIGNAL(publishStateChanged(Tp::Contact::PresenceState, QString)),
            SLOT(onPublishStateChanged(Tp::Contact::PresenceState)));
    // FIXME: Add support to the ontology for the message QString.
    connect(m_contact.data(),
            SIGNAL(blockStatusChanged(bool)),
            SLOT(onBlockStatusChanged(bool)));
    connect(m_contact.data(),
            SIGNAL(avatarDataChanged(Tp::AvatarData)),
            SLOT(onAvatarDataChanged(Tp::AvatarData)));
    // FIXME: Connect to any other signals of sync-worthy properties here.

    // Emit a signal to notify the controller that a new contact has been created.
    emit created(m_contact->id());

    // Synthesize all the properties being changed
    // FIXME: Make sure all needed properties are included
    onPresenceChanged(m_contact->presence());
    onAliasChanged(m_contact->alias());
    onAddedToGroup(QString());  // Arg is ignored.
    // No need to call onRemovedFromGroup too since does the same as added.
    onCapabilitiesChanged(m_contact->capabilities());
    onSubscriptionStateChanged(m_contact->subscriptionState());
    onPublishStateChanged(m_contact->publishState());
    onBlockStatusChanged(m_contact->isBlocked());

    // Become ready asynchronously with the avatar data, since this may take some time and we
    // don't want to delay the rest of the contact's attributes being ready.
    // Fire and forget because we can't do anything even if this fails.
    Tp::Features f;
    f << Tp::Contact::FeatureAvatarData
      << Tp::Contact::FeatureAvatarToken;
    m_contact->manager()->upgradeContacts(QList<Tp::ContactPtr>() << m_contact, f);
}

Contact::~Contact()
{
    //kDebug();
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
//    kDebug() << "On added to group " << group;

    emit groupsChanged(m_contact->id(), m_contact->groups());
}

void Contact::onRemovedFromGroup(const QString &group)
{
//    kDebug() << "On removed from group " << group;

    emit groupsChanged(m_contact->id(), m_contact->groups());
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
    emit capabilitiesChanged(m_contact->id(), m_contact->manager()->connection(), capabilities);
}

void Contact::onAvatarDataChanged(const Tp::AvatarData &avatar)
{
    emit avatarChanged(m_contact->id(), avatar);
}


#include "contact.moc"

