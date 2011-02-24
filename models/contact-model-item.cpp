/*
 * Contacts model item, represents a contact in the contactlist tree
 * This file is based on TelepathyQt4Yell Models
 *
 * Copyright (C) 2010 Collabora Ltd. <http://www.collabora.co.uk/>
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

#include <QImage>

#include <TelepathyQt4/AvatarData>
#include <TelepathyQt4/ContactCapabilities>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/RequestableChannelClassSpec>

#include "accounts-model.h"
#include "contact-model-item.h"

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
}

ContactModelItem::~ContactModelItem()
{
    delete mPriv;
}

QVariant ContactModelItem::data(int role) const
{
    switch (role)
    {
        case AccountsModel::ItemRole:
            return QVariant::fromValue(
                const_cast<QObject *>(
                    static_cast<const QObject *>(this)));
        case AccountsModel::IdRole:
            return mPriv->mContact->id();
        case Qt::DisplayRole:
        case AccountsModel::AliasRole:
            return mPriv->mContact->alias();
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
            return mPriv->mContact->capabilities().streamedMediaCalls();
        case AccountsModel::AudioCallCapabilityRole:
            return mPriv->mContact->capabilities().streamedMediaAudioCalls();
        case AccountsModel::VideoCallCapabilityRole:
            return mPriv->mContact->capabilities().streamedMediaVideoCalls();
        case AccountsModel::UpgradeCallCapabilityRole:
            return mPriv->mContact->capabilities().upgradingStreamedMediaCalls();
        case AccountsModel::FileTransferCapabilityRole: {
            foreach (const Tp::RequestableChannelClassSpec &rccSpec, mPriv->mContact->capabilities().allClassSpecs()) {
                if (rccSpec.supports(Tp::RequestableChannelClassSpec::fileTransfer())) {
                    return true;
                }
            }
            return false;
        }
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
    emit changed(this);
}

Tp::ContactPtr ContactModelItem::contact() const
{
    return mPriv->mContact;
}