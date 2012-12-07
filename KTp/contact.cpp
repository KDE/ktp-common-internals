/*
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

#include "contact.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ContactCapabilities>

#include "capabilities-hack-private.h"

KTp::Contact::Contact(Tp::ContactManager *manager, const Tp::ReferencedHandles &handle, const Tp::Features &requestedFeatures, const QVariantMap &attributes)
    : Tp::Contact(manager, handle, requestedFeatures, attributes)
{
    connect(manager->connection().data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)), SIGNAL(invalidated()));
}

KTp::Presence KTp::Contact::presence() const
{
    return KTp::Presence(Tp::Contact::presence());
}

bool KTp::Contact::audioCallCapability() const
{
    if (! manager()->connection()) {
        return false;
    }
    Tp::ConnectionPtr connection = manager()->connection();
    if (connection) {
        bool contactCanStreamAudio = CapabilitiesHackPrivate::audioCalls(
                    capabilities(), connection->cmName());
        bool selfCanStreamAudio = CapabilitiesHackPrivate::audioCalls(
                    connection->selfContact()->capabilities(), connection->cmName());
        return contactCanStreamAudio && selfCanStreamAudio;
    }
    return false;
}

bool KTp::Contact::videoCallCapability() const
{
    if (! manager()->connection()) {
        return false;
    }
    Tp::ConnectionPtr connection = manager()->connection();
    if (connection) {
        bool contactCanStreamVideo = CapabilitiesHackPrivate::videoCalls(
                    capabilities(), connection->cmName());
        bool selfCanStreamVideo = CapabilitiesHackPrivate::videoCalls(
                    connection->selfContact()->capabilities(), connection->cmName());
        return contactCanStreamVideo && selfCanStreamVideo;
    }

    return false;
}

bool KTp::Contact::fileTransferCapability()  const
{
    if (! manager()->connection()) {
        return false;
    }
    if (manager()->connection()) {
        bool contactCanHandleFiles = capabilities().fileTransfers();
        bool selfCanHandleFiles = manager()->connection()->selfContact()->capabilities().fileTransfers();
        return contactCanHandleFiles && selfCanHandleFiles;
    }

    return false;
}

QStringList KTp::Contact::clientTypes() const
{
    /* Temporary workaround for upstream bug https://bugs.freedesktop.org/show_bug.cgi?id=55883)
     * Close https://bugs.kde.org/show_bug.cgi?id=308217 when fixed upstream */
    if (Tp::Contact::presence().type() == Tp::ConnectionPresenceTypeOffline) {
        return QStringList();
    }

    return Tp::Contact::clientTypes();
}

