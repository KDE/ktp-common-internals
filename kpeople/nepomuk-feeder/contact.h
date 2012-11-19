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

#ifndef TELEPATHY_NEPOMUK_SERVICE_CONTACT_H
#define TELEPATHY_NEPOMUK_SERVICE_CONTACT_H

#include <QtCore/QObject>

#include <TelepathyQt/Connection>
#include <TelepathyQt/Contact>
#include <TelepathyQt/Presence>

/**
 * This class wraps one Telepathy Contact.
 */
class Contact : public QObject
{
    Q_OBJECT

public:
    explicit Contact(const Tp::ContactPtr &contact, QObject *parent = 0);
    ~Contact();

    void init();
    void shutdown();

Q_SIGNALS:
    void created(const QString &id);
    void contactDestroyed(const QString &id, const Tp::ContactPtr &contact);
    void aliasChanged(const QString &id, const QString &alias);
    void presenceChanged(const QString &id, const Tp::SimplePresence &presence);
    void groupsChanged(const QString &id, const QStringList &groups);
    void blockStatusChanged(const QString &id, bool blocked);
    void publishStateChanged(const QString &id, const Tp::Contact::PresenceState &state);
    void subscriptionStateChanged(const QString &id, const Tp::Contact::PresenceState &state);
    void capabilitiesChanged(const QString &id, const Tp::ConnectionPtr &connection, const Tp::ContactCapabilities &capabilities);
    void avatarChanged(const QString &id, const Tp::AvatarData &avatar);

private Q_SLOTS:
    void onAliasChanged(const QString &alias);
    void onPresenceChanged(const Tp::Presence &presence);
    void onAddedToGroup(const QString &group);
    void onRemovedFromGroup(const QString &group);
    void onBlockStatusChanged(bool blocked);
    void onPublishStateChanged(Tp::Contact::PresenceState state);
    void onSubscriptionStateChanged(Tp::Contact::PresenceState state);
    void onCapabilitiesChanged(const Tp::ContactCapabilities &capabilities);
    void onAvatarDataChanged(const Tp::AvatarData &avatar);

private:
    Q_DISABLE_COPY(Contact);

    Tp::ContactPtr m_contact;
};


#endif // Header guard

