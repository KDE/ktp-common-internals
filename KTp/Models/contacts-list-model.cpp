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

#include "contacts-list-model.h"

#include <TelepathyQt/Account>
#include <TelepathyQt/Contact>
#include <TelepathyQt/ContactCapabilities>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ContactManager>

#include "contact.h"
#include "presence.h"
#include "types.h"

class KTp::ContactsListModel::Private
{
public:
    QList<Tp::ContactPtr> contacts;
    KTp::GlobalContactManager *contactManager;
};


KTp::ContactsListModel::ContactsListModel(QObject *parent) :
    QAbstractListModel(parent),
    d(new KTp::ContactsListModel::Private())
{
    d->contactManager = 0;

    QHash<int, QByteArray> roles = roleNames();
    roles[KTp::RowTypeRole]= "type";
    roles[KTp::IdRole]= "id";

    roles[KTp::ContactRole]= "contact";
    roles[KTp::AccountRole]= "account";

    roles[KTp::ContactClientTypesRole]= "clientTypes";
    roles[KTp::ContactAvatarPathRole]= "avatar";
    roles[KTp::ContactAvatarPixmapRole]="avatarPixmap";
    roles[KTp::ContactGroupsRole]= "groups";
    roles[KTp::ContactPresenceMessageRole]= "presenceMessage";
    roles[KTp::ContactPresenceTypeRole]= "presenceType";
    roles[KTp::ContactPresenceIconRole]= "presenceIcon";
    roles[KTp::ContactSubscriptionStateRole]= "subscriptionState";
    roles[KTp::ContactPublishStateRole]= "publishState";
    roles[KTp::ContactIsBlockedRole]= "blocked";
    roles[KTp::ContactCanTextChatRole]= "textChat";
    roles[KTp::ContactCanFileTransferRole]= "fileTransfer";
    roles[KTp::ContactCanAudioCallRole]= "audioCall";
    roles[KTp::ContactCanVideoCallRole]= "videoCall";
    roles[KTp::ContactTubesRole]= "tubes";
    setRoleNames(roles);
}

KTp::ContactsListModel::~ContactsListModel()
{
    delete d;
}

void KTp::ContactsListModel::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    d->contactManager = new KTp::GlobalContactManager(accountManager, this);
    connect(d->contactManager, SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts)), SLOT(onContactsChanged(Tp::Contacts,Tp::Contacts)));
}

int KTp::ContactsListModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return d->contacts.size();
    } else {
        return 0;
    }
}

QVariant KTp::ContactsListModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    if (row >=0 && row < d->contacts.size()) {
        const KTp::ContactPtr contact = KTp::ContactPtr::qObjectCast(d->contacts[row]);
        Q_ASSERT_X(!contact.isNull(), "KTp::ContactListModel::data()",
                   "Failed to cast Tp::ContactPtr to KTp::ContactPtr. Are you using KTp::ContactFactory?");

        switch (role) {
        case KTp::RowTypeRole:
            return KTp::ContactRowType;
        case Qt::DisplayRole:
            return contact->alias();
       case KTp::IdRole:
            return contact->id();

        case KTp::ContactRole:
            return QVariant::fromValue(contact);
        case KTp::AccountRole:
            return QVariant::fromValue(d->contactManager->accountForContact(contact));

        case KTp::ContactClientTypesRole:
            return contact->clientTypes();
        case KTp::ContactAvatarPathRole:
            return contact->avatarData().fileName;
        case KTp::ContactAvatarPixmapRole:
            return contact->avatarPixmap();
        case KTp::ContactGroupsRole:
            return contact->groups();

        case KTp::ContactPresenceNameRole:
            return contact->presence().displayString();
        case KTp::ContactPresenceMessageRole:
            return contact->presence().statusMessage();
        case KTp::ContactPresenceTypeRole:
            return contact->presence().type();
        case KTp::ContactPresenceIconRole:
            return contact->presence().iconName();

        case KTp::ContactSubscriptionStateRole:
            return contact->subscriptionState();
          case KTp::ContactPublishStateRole:
            return contact->publishState();
        case KTp::ContactIsBlockedRole:
            return contact->isBlocked();

        case KTp::ContactCanTextChatRole:
            return true; //FIXME
          case KTp::ContactCanFileTransferRole:
            return contact->fileTransferCapability();
        case KTp::ContactCanAudioCallRole:
            return contact->audioCallCapability();
        case KTp::ContactCanVideoCallRole:
            return contact->videoCallCapability();
        case KTp::ContactTubesRole:
            //FIXME this does not check own selfContact caps.
            return QStringList() << contact->capabilities().streamTubeServices()
                                 << contact->capabilities().dbusTubeServices();

        default:
            break;
        }
    }
    return QVariant();
}

void KTp::ContactsListModel::onContactsChanged(const Tp::Contacts &added, const Tp::Contacts &removed)
{
    //add contacts.

    Q_FOREACH(const Tp::ContactPtr &contact_uncasted, added) {
        KTp::ContactPtr contact = KTp::ContactPtr::qObjectCast(contact_uncasted);

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
        connect(contact.data(),
                SIGNAL(addedToGroup(QString)),
                SLOT(onChanged()));
        connect(contact.data(),
                SIGNAL(removedFromGroup(QString)),
                SLOT(onChanged()));

        connect(contact.data(),
                SIGNAL(invalidated()),
                SLOT(onConnectionDropped()));
    }

    if (added.size() > 0) {
        beginInsertRows(QModelIndex(), d->contacts.size(), d->contacts.size() + added.size() -1);
        d->contacts.append(added.toList());
        endInsertRows();
    }

    //remove contacts
    Q_FOREACH(const Tp::ContactPtr &contact, removed) {
        int row = d->contacts.indexOf(contact);
        if (row >= 0) { //if contact found in list
            beginRemoveRows(QModelIndex(), row, row);
            d->contacts.removeOne(contact);
            endRemoveRows();
        }
    }
}

void KTp::ContactsListModel::onChanged()
{
    KTp::ContactPtr contact(qobject_cast<KTp::Contact*>(sender()));
    int row = d->contacts.indexOf(contact);
    if (row > 0) {
        QModelIndex index = createIndex(row, 0);
        dataChanged(index, index);
    }
}

void KTp::ContactsListModel::onConnectionDropped()
{
    KTp::ContactPtr contact(qobject_cast<KTp::Contact*>(sender()));
    onContactsChanged(Tp::Contacts(), Tp::Contacts() << contact);
}

