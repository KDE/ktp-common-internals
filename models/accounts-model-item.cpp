/*
 * Accounts model item, represents an account in the contactlist tree
 * This file is based on TelepathyQt4Yell Models
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

#include "accounts-model-item.h"

#include <TelepathyQt4/Account>
#include <TelepathyQt4/ContactManager>

#include "accounts-model.h"
#include "contact-model-item.h"

struct AccountsModelItem::Private
{
    Private(const Tp::AccountPtr &account)
        : mAccount(account),
          mOnlineCount(0)
    {
    }

    void setStatus(const QString &value);
    void setStatusMessage(const QString &value);

    Tp::AccountPtr mAccount;
    int mOnlineCount;
};

void AccountsModelItem::Private::setStatus(const QString &value)
{
    Tp::Presence presence = mAccount->currentPresence().barePresence();
    presence.setStatus(Tp::ConnectionPresenceTypeUnset, value, QString());
    mAccount->setRequestedPresence(presence);
}

void AccountsModelItem::Private::setStatusMessage(const QString &value)
{
    Tp::Presence presence = mAccount->currentPresence().barePresence();
    presence.setStatus(Tp::ConnectionPresenceTypeUnset, QString(), value);
    mAccount->setRequestedPresence(presence);
}

AccountsModelItem::AccountsModelItem(const Tp::AccountPtr &account)
    : mPriv(new Private(account))
{
    if (!mPriv->mAccount->connection().isNull()) {
        QTimer::singleShot(0, this, SLOT(onNewConnection()));
    }

    connect(mPriv->mAccount.data(),
            SIGNAL(removed()),
            SLOT(onRemoved()));
    connect(mPriv->mAccount.data(),
            SIGNAL(serviceNameChanged(QString)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(profileChanged(Tp::ProfilePtr)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(iconNameChanged(QString)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(nicknameChanged(QString)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(normalizedNameChanged(QString)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(validityChanged(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(stateChanged(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(capabilitiesChanged(Tp::ConnectionCapabilities)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(connectsAutomaticallyPropertyChanged(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(parametersChanged(QVariantMap)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(changingPresence(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(automaticPresenceChanged(Tp::Presence)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(currentPresenceChanged(Tp::Presence)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(requestedPresenceChanged(Tp::Presence)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(onlinenessChanged(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(avatarChanged(Tp::Avatar)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(onlinenessChanged(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
            SLOT(onStatusChanged(Tp::ConnectionStatus)));
    connect(mPriv->mAccount.data(),
            SIGNAL(connectionChanged(Tp::ConnectionPtr)),
            SLOT(onConnectionChanged(Tp::ConnectionPtr)));
}

AccountsModelItem::~AccountsModelItem()
{
    delete mPriv;
}

QVariant AccountsModelItem::data(int role) const
{
    switch (role) {
    case AccountsModel::ItemRole:
        return QVariant::fromValue((AccountsModelItem*)this);
    case AccountsModel::IdRole:
        return mPriv->mAccount->uniqueIdentifier();
    case AccountsModel::AvatarRole:
        return QVariant::fromValue(mPriv->mAccount->avatar());
    case AccountsModel::ValidRole:
        return mPriv->mAccount->isValid();
    case AccountsModel::EnabledRole:
        return mPriv->mAccount->isEnabled();
    case AccountsModel::ConnectionManagerNameRole:
        return mPriv->mAccount->cmName();
    case AccountsModel::ProtocolNameRole:
        return mPriv->mAccount->protocolName();
    case AccountsModel::DisplayNameRole:
    case Qt::DisplayRole:
        return mPriv->mAccount->displayName();
    case AccountsModel::IconRole:
        return mPriv->mAccount->iconName();
    case AccountsModel::NicknameRole:
        return mPriv->mAccount->nickname();
    case AccountsModel::ConnectsAutomaticallyRole:
        return mPriv->mAccount->connectsAutomatically();
    case AccountsModel::ChangingPresenceRole:
        return mPriv->mAccount->isChangingPresence();
    case AccountsModel::AutomaticPresenceRole:
        return mPriv->mAccount->automaticPresence().status();
    case AccountsModel::AutomaticPresenceTypeRole:
        return mPriv->mAccount->automaticPresence().type();
    case AccountsModel::AutomaticPresenceStatusMessageRole:
        return mPriv->mAccount->automaticPresence().statusMessage();
    case AccountsModel::CurrentPresenceRole:
        return mPriv->mAccount->currentPresence().status();
    case AccountsModel::CurrentPresenceTypeRole:
        return mPriv->mAccount->currentPresence().type();
    case AccountsModel::CurrentPresenceStatusMessageRole:
        return mPriv->mAccount->currentPresence().statusMessage();
    case AccountsModel::RequestedPresenceRole:
        return mPriv->mAccount->requestedPresence().status();
    case AccountsModel::RequestedPresenceTypeRole:
        return mPriv->mAccount->requestedPresence().type();
    case AccountsModel::RequestedPresenceStatusMessageRole:
        return mPriv->mAccount->requestedPresence().statusMessage();
    case AccountsModel::ConnectionStatusRole:
        return mPriv->mAccount->connectionStatus();
    case AccountsModel::ConnectionStatusReasonRole:
        return mPriv->mAccount->connectionStatusReason();
    case AccountsModel::TotalUsersCountRole:
        return size();
    case AccountsModel::OnlineUsersCountRole:
        return mPriv->mOnlineCount;
    default:
        return QVariant();
    }
}

bool AccountsModelItem::setData(int role, const QVariant &value)
{
    switch (role) {
    case AccountsModel::EnabledRole:
        setEnabled(value.toBool());
        return true;
    case AccountsModel::RequestedPresenceRole:
        mPriv->setStatus(value.toString());
        return true;
    case AccountsModel::RequestedPresenceStatusMessageRole:
        mPriv->setStatusMessage(value.toString());
        return true;
    case AccountsModel::NicknameRole:
        setNickname(value.toString());
        return true;
    case AccountsModel::AvatarRole:
        mPriv->mAccount->setAvatar(value.value<Tp::Avatar>());
        return true;
    default:
        return false;
    }
}

Tp::AccountPtr AccountsModelItem::account() const
{
    return mPriv->mAccount;
}

void AccountsModelItem::setEnabled(bool value)
{
    mPriv->mAccount->setEnabled(value);
}

void AccountsModelItem::setNickname(const QString &value)
{
    mPriv->mAccount->setNickname(value);
}

void AccountsModelItem::setAutomaticPresence(int type, const QString &status, const QString &statusMessage)
{
    Tp::Presence presence;
    presence.setStatus((Tp::ConnectionPresenceType) type, status, statusMessage);
    mPriv->mAccount->setAutomaticPresence(presence);
}

void AccountsModelItem::setRequestedPresence(int type, const QString &status, const QString &statusMessage)
{
    Tp::Presence presence;
    presence.setStatus((Tp::ConnectionPresenceType) type, status, statusMessage);
    mPriv->mAccount->setRequestedPresence(presence);
}

void AccountsModelItem::onRemoved()
{
    int index = parent()->indexOf(this);
    emit childrenRemoved(parent(), index, index);
}

void AccountsModelItem::onChanged()
{
    emit changed(this);
}

void AccountsModelItem::onContactsChanged(const Tp::Contacts &addedContacts,
        const Tp::Contacts &removedContacts)
{
    foreach (const Tp::ContactPtr &contact, removedContacts) {
        for (int i = 0; i < size(); ++i) {
            ContactModelItem *item = qobject_cast<ContactModelItem *>(childAt(i));
            if (item->contact() == contact) {
                emit childrenRemoved(this, i, i);
                break;
            }
        }
    }

    // get the list of contact ids in the children
    QStringList idList;
    int numElems = size();
    for (int i = 0; i < numElems; ++i) {
        ContactModelItem *item = qobject_cast<ContactModelItem *>(childAt(i));
        if (item) {
            idList.append(item->contact()->id());
        }
    }

    QList<TreeNode *> newNodes;
    foreach (const Tp::ContactPtr &contact, addedContacts) {
        if (!idList.contains(contact->id())) {
            newNodes.append(new ContactModelItem(contact));
        }
    }
    emit childrenAdded(this, newNodes);

    countOnlineContacts();
}

void AccountsModelItem::onStatusChanged(Tp::ConnectionStatus status)
{
    onChanged();

    emit connectionStatusChanged(mPriv->mAccount->uniqueIdentifier(), status);
}

void AccountsModelItem::onNewConnection()
{
  onConnectionChanged(mPriv->mAccount->connection());
}

void AccountsModelItem::onConnectionChanged(const Tp::ConnectionPtr &connection)
{
    onChanged();

    // if the connection is invalid or disconnected, clear the contacts list
    if (connection.isNull()
            || !connection->isValid()
            || connection->status() == Tp::ConnectionStatusDisconnected) {
        if (size() > 0) {
            emit childrenRemoved(this, 0, size() - 1);
        }
        return;
    }

    Tp::ContactManagerPtr manager = connection->contactManager();

    connect(manager.data(),
            SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts,
                                           Tp::Channel::GroupMemberChangeDetails)),
            SLOT(onContactsChanged(Tp::Contacts,Tp::Contacts)));

    connect(manager.data(),
            SIGNAL(stateChanged(Tp::ContactListState)),
            SLOT(onContactManagerStateChanged(Tp::ContactListState)));
    onContactManagerStateChanged(manager->state());
}

void AccountsModelItem::onContactManagerStateChanged(Tp::ContactListState state)
{
    if (state == Tp::ContactListStateSuccess) {
        clearContacts();
        addKnownContacts();
    }
}



void AccountsModelItem::clearContacts()
{
    if (!mPriv->mAccount->connection().isNull() &&
        mPriv->mAccount->connection()->isValid()) {
        Tp::ContactManagerPtr manager = mPriv->mAccount->connection()->contactManager();
        Tp::Contacts contacts = manager->allKnownContacts();

        // remove the items no longer present
        for (int i = 0; i < size(); ++i) {
            bool exists = false;
            ContactModelItem *item = qobject_cast<ContactModelItem *>(childAt(i));
            if (item) {
                Tp::ContactPtr itemContact = item->contact();
                if (contacts.contains(itemContact)) {
                    exists = true;
                }
            }
            if (!exists) {
                emit childrenRemoved(this, i, i);
            }
        }
    }
}

void AccountsModelItem::addKnownContacts()
{
    // reload the known contacts if it has a connection
    QList<TreeNode *> newNodes;
    if (!mPriv->mAccount->connection().isNull() &&
        mPriv->mAccount->connection()->isValid()) {
        Tp::ContactManagerPtr manager = mPriv->mAccount->connection()->contactManager();
        Tp::Contacts contacts = manager->allKnownContacts();

        // get the list of contact ids in the children
        QStringList idList;
        int numElems = size();

        for (int i = 0; i < numElems; ++i) {
            ContactModelItem *item = qobject_cast<ContactModelItem *>(childAt(i));
            if (item) {
                idList.append(item->contact()->id());
            }
        }

        // only add the contact item if it is new
        foreach (const Tp::ContactPtr &contact, contacts) {
            if (!idList.contains(contact->id())) {
                newNodes.append(new ContactModelItem(contact));
            }
        }
    }

    if (newNodes.count() > 0) {
        emit childrenAdded(this, newNodes);
    }

    countOnlineContacts();
}

void AccountsModelItem::countOnlineContacts()
{
    int tmpCounter = 0;
    for (int i = 0; i < size(); ++i) {
        ContactModelItem* contactNode = qobject_cast<ContactModelItem*>(childAt(i));
        Q_ASSERT(contactNode);
        if (contactNode->data(AccountsModel::PresenceTypeRole).toUInt() != Tp::ConnectionPresenceTypeOffline
            && contactNode->data(AccountsModel::PresenceTypeRole).toUInt() != Tp::ConnectionPresenceTypeUnknown) {
            tmpCounter++;
        }
    }

    mPriv->mOnlineCount = tmpCounter;
}

#include "accounts-model-item.moc"
