/*
 * Contacts model item, represents a contact in the contactlist tree
 * This file is based on TelepathyQtYell Models
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
 * Copyright (C) 2011 Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "contact-model-item.h"
#include "accounts-model.h"
#include "capabilities-hack-private.h"
#include "../service-availability-checker.h"
#include "presence.h"

#include <QImage>

#include <KGlobal>

#include <TelepathyQt/AvatarData>
#include <TelepathyQt/ContactCapabilities>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/RequestableChannelClassSpec>



K_GLOBAL_STATIC_WITH_ARGS(KTp::ServiceAvailabilityChecker, s_krfbAvailableChecker,
                          (QLatin1String("org.freedesktop.Telepathy.Client.krfb_rfb_handler")));


struct ContactModelItem::Private
{
    Private(const Tp::ContactPtr &contact)
        : mContact(contact)
    {
    }

    Tp::ContactPtr mContact;
};

ContactModelItem::ContactModelItem(const Tp::ContactPtr &contact)
    : mPriv(new Private(contact))
{
    //This effectively constructs the s_krfbAvailableChecker object the first
    //time that this code is executed. This is to start the d-bus query early, so
    //that data are available when we need them later in desktopSharingCapability()
    (void) s_krfbAvailableChecker.operator->();

    connect(contact.data(),
            SIGNAL(aliasChanged(QString)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(avatarTokenChanged(QString)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(avatarDataChanged(Tp::AvatarData)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SLOT(onChanged()));
    connect(contact->manager()->connection()->selfContact().data(),
            SIGNAL(capabilitiesChanged(Tp::ContactCapabilities)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(capabilitiesChanged(Tp::ContactCapabilities)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(locationUpdated(Tp::LocationInfo)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(infoFieldsChanged(Tp::Contact::InfoFields)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(publishStateChanged(Tp::Contact::PresenceState,QString)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(blockStatusChanged(bool)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(clientTypesChanged(QStringList)),
            SLOT(onChanged()));
}

ContactModelItem::~ContactModelItem()
{
    delete mPriv;
}

QVariant ContactModelItem::data(int role) const
{
    switch (role) {
    case AccountsModel::ItemRole:
        return QVariant::fromValue((ContactModelItem*)this);
    case AccountsModel::IdRole:
        return mPriv->mContact->id();
    case Qt::DisplayRole:
    case AccountsModel::AliasRole:
        return mPriv->mContact->alias();
    case AccountsModel::PresenceRole:
        return QVariant::fromValue(KTp::Presence(mPriv->mContact->presence()));
    case AccountsModel::PresenceIconRole:
        return QIcon(KTp::Presence(mPriv->mContact->presence()).icon());
    case AccountsModel::PresenceStatusRole:
        return mPriv->mContact->presence().status();
    case AccountsModel::PresenceTypeRole:
        return mPriv->mContact->presence().type();
    case AccountsModel::PresenceMessageRole:
        return mPriv->mContact->presence().statusMessage();
    case AccountsModel::SubscriptionStateRole:
        return mPriv->mContact->subscriptionState();
    case AccountsModel::PublishStateRole:
        return mPriv->mContact->publishState();
    case AccountsModel::BlockedRole:
        return mPriv->mContact->isBlocked();
    case AccountsModel::GroupsRole:
        return mPriv->mContact->groups();
    case AccountsModel::AvatarRole:
        return mPriv->mContact->avatarData().fileName;
    case Qt::DecorationRole:
        return QImage(mPriv->mContact->avatarData().fileName);
    case AccountsModel::TextChatCapabilityRole:
        return mPriv->mContact->capabilities().textChats();
    case AccountsModel::MediaCallCapabilityRole:
        return audioCallCapability() || videoCallCapability();
    case AccountsModel::AudioCallCapabilityRole:
        return audioCallCapability();
    case AccountsModel::VideoCallCapabilityRole:
        return videoCallCapability();
    case AccountsModel::UpgradeCallCapabilityRole:
        return mPriv->mContact->capabilities().upgradingCalls();
    case AccountsModel::FileTransferCapabilityRole:
        return fileTransferCapability();
    case AccountsModel::DesktopSharingCapabilityRole:
        return desktopSharingCapability();
    case AccountsModel::SSHContactCapabilityRole:
        return sshContactCapability();
    case AccountsModel::ClientTypesRole:
        return mPriv->mContact->clientTypes();
    default:
        break;
    }

    return QVariant();
}

bool ContactModelItem::setData(int role, const QVariant &value)
{
    switch (role) {
    case AccountsModel::PublishStateRole: {
        Tp::Contact::PresenceState state;
        state = (Tp::Contact::PresenceState) value.toInt();
        switch (state) {
        case Tp::Contact::PresenceStateYes:
            // authorize the contact and request its presence publication
            mPriv->mContact->authorizePresencePublication();
            mPriv->mContact->requestPresenceSubscription();
            return true;
        case Tp::Contact::PresenceStateNo: {
            // reject the presence publication and remove the contact
            mPriv->mContact->removePresencePublication();
            QList<Tp::ContactPtr> contacts;
            contacts << mPriv->mContact;
            mPriv->mContact->manager()->removeContacts(contacts);
            return true;
        }
        default:
            return false;
        }
    }
    default:
        return false;
    }
}

void ContactModelItem::onChanged()
{
    Q_EMIT changed(this);
}

Tp::ContactPtr ContactModelItem::contact() const
{
    return mPriv->mContact;
}

//return true if both you and the contact can handle audio calls.
bool ContactModelItem::audioCallCapability() const
{
    Tp::ConnectionPtr connection = mPriv->mContact->manager()->connection();
    if (connection) {
        bool contactCanStreamAudio = CapabilitiesHackPrivate::audioCalls(
                mPriv->mContact->capabilities(), connection->cmName());
        bool selfCanStreamAudio = CapabilitiesHackPrivate::audioCalls(
                connection->selfContact()->capabilities(), connection->cmName());
        return contactCanStreamAudio && selfCanStreamAudio;
    }

    return false;
}

bool ContactModelItem::videoCallCapability() const
{
    Tp::ConnectionPtr connection = mPriv->mContact->manager()->connection();
    if (connection) {
        bool contactCanStreamVideo = CapabilitiesHackPrivate::videoCalls(
                mPriv->mContact->capabilities(), connection->cmName());
        bool selfCanStreamVideo = CapabilitiesHackPrivate::videoCalls(
                connection->selfContact()->capabilities(), connection->cmName());
        return contactCanStreamVideo && selfCanStreamVideo;
    }

    return false;
}

bool ContactModelItem::fileTransferCapability() const
{
    if (mPriv->mContact->manager()->connection()) {
        bool contactCanHandleFiles = mPriv->mContact->capabilities().fileTransfers();
        bool selfCanHandleFiles = mPriv->mContact->manager()->connection()->selfContact()->capabilities().fileTransfers();
        return contactCanHandleFiles && selfCanHandleFiles;
    }

    return false;
}

bool ContactModelItem::desktopSharingCapability() const
{
    bool contactCanHandleRfb = mPriv->mContact->capabilities().streamTubes(QLatin1String("rfb"));
    bool selfCanHandleRfb = s_krfbAvailableChecker->isAvailable();
    return contactCanHandleRfb && selfCanHandleRfb;
}

bool ContactModelItem::sshContactCapability() const
{
    bool contactCanHandleSSHContact = mPriv->mContact->capabilities().streamTubes(QLatin1String("x-ssh-contact"));
    bool selfCanHandleSSHContact = true; // FIXME Check if ssh-contact client is installed
    return contactCanHandleSSHContact && selfCanHandleSSHContact;
}

#include "contact-model-item.moc"
