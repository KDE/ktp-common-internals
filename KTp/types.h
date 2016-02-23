/*
 * Common enums and types in KTp
 *
 * Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
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

#ifndef KTP_TYPES_H
#define KTP_TYPES_H

#include "contact.h"

#include "core.h"

#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>

#include <QVariant>

//this is deliberately the wrong namespace for backwards compatability, we will change it when the roles are sorted.
namespace KTp
{
    enum RowType {
        ContactRowType,
        PersonRowType,
        AccountRowType,
        GroupRowType,
        RoomRowType,
        MergeRowType,
        UserRowType = 1000
    };

    enum ContactsModelRole {
        // general roles
        RowTypeRole = Qt::UserRole, //returns one of KTp::ContactRowType, KTp::PersonRowType, KTp::GroupRowType, KTp::AccountRowType
        IdRole, //returns Contact ID, Account UID, or group ID (group name or "_ungrouped")
        PersonIdRole, ///< id of the corresponding contact/person resource in kpeople

        //telepathy roles
        ContactRole = Qt::UserRole + 1000,  ///<return Tp::ContactPtr
        AccountRole, ///< return Tp::AccountPtr
        ChannelRole, ///< return Tp::ChannelPtr

        //contact/person roles
        ContactClientTypesRole = Qt::UserRole + 2000, ///< stringlist. See Tp::Contact::ClientTypes
        ContactAvatarPathRole, ///<string. path to avatar file
        ContactAvatarPixmapRole, ///< QPixmap the pixmap that shall be use as avatar image
        ContactGroupsRole, ///< stringlist. of all groups contact is in

        ContactPresenceNameRole,
        ContactPresenceMessageRole,
        ContactPresenceTypeRole,
        ContactPresenceIconRole,

        ContactSubscriptionStateRole, ///< enum of type Tp::Contact::PresenceState
        ContactPublishStateRole, ///< enum of type Tp::Contact::PresenceState
        ContactIsBlockedRole, ///< bool, true if contact is blocked

        ContactHasTextChannelRole, ///< bool, returns true if a text channel is active for this contact
        ContactUnreadMessageCountRole, ///< int. the number of unread messages in active channels with this contact
        ContactLastMessageRole, ///string, the last message to/from this contact in an active chat
        ContactLastMessageDirectionRole, //enum KTp::Message::MessageDirection direction of last message

        ContactCanTextChatRole, ///< bool. You and contact can both text chat
        ContactCanFileTransferRole, ///< bool. You and contact can both file transfer
        ContactCanAudioCallRole, ///< bool. You and contact can both audio call
        ContactCanVideoCallRole, ///< bool. You and contact can both video call
        ContactTubesRole, ///< stringlist. common supported dbus + stream services between you and contact

        ContactUriRole,
        ContactVCardRole, ///< VCard of the contact in KContacts::Addresse format; KPeople only at the moment

        //heading roles
        HeaderTotalUsersRole = Qt::UserRole  + 3000,
        HeaderOnlineUsersRole,

        CustomRole = Qt::UserRole + 4000 // a placemark for custom roles in inherited models
    };
}

static const QString S_KPEOPLE_PROPERTY_ACCOUNT_PATH = QStringLiteral("telepathy-accountPath");
static const QString S_KPEOPLE_PROPERTY_ACCOUNT_DISPLAY_NAME = QStringLiteral("telepathy-accountDisplayName");
static const QString S_KPEOPLE_PROPERTY_CONTACT_ID = QStringLiteral("telepathy-contactId");
static const QString S_KPEOPLE_PROPERTY_CONTACT_URI = QStringLiteral("telepathy-contactUri");
static const QString S_KPEOPLE_PROPERTY_PRESENCE = QStringLiteral("telepathy-presence");
static const QString S_KPEOPLE_PROPERTY_IS_BLOCKED = QStringLiteral("telepathy-isBlocked");

const QHash<Tp::ConnectionPresenceType, QString> s_presenceStrings = {
    { Tp::ConnectionPresenceTypeUnset, QString() },
    { Tp::ConnectionPresenceTypeOffline, QStringLiteral("offline") },
    { Tp::ConnectionPresenceTypeAvailable, QStringLiteral("available") },
    { Tp::ConnectionPresenceTypeAway, QStringLiteral("away") },
    { Tp::ConnectionPresenceTypeExtendedAway, QStringLiteral("xa") },
    { Tp::ConnectionPresenceTypeHidden, QStringLiteral("hidden") }, //or 'offline' ?
    { Tp::ConnectionPresenceTypeBusy, QStringLiteral("busy") },
    { Tp::ConnectionPresenceTypeUnknown, QStringLiteral("unknown") },
    { Tp::ConnectionPresenceTypeError, QStringLiteral("error") }
};

Q_DECLARE_METATYPE(Tp::AccountPtr)
Q_DECLARE_METATYPE(KTp::ContactPtr)
Q_DECLARE_METATYPE(Tp::AccountManagerPtr);
Q_DECLARE_METATYPE(Tp::ConnectionPtr);
Q_DECLARE_METATYPE(Tp::TextChannelPtr);
Q_DECLARE_METATYPE(Tp::ChannelPtr);


#endif // KTP_TYPES_H
