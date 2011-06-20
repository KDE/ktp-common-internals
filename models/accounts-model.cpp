/*
 * Accounts and contacts model
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

#include "accounts-model.h"

#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingReady>

#include "accounts-model-item.h"
#include "contact-model-item.h"

struct AccountsModel::Private
{
    Private(const Tp::AccountManagerPtr &am)
        : mAM(am)
    {
    }

    TreeNode *node(const QModelIndex &index) const;

    Tp::AccountManagerPtr mAM;
    TreeNode *mTree;
};

TreeNode *AccountsModel::Private::node(const QModelIndex &index) const
{
    TreeNode *node = reinterpret_cast<TreeNode *>(index.internalPointer());
    return node ? node : mTree;
}

AccountsModel::AccountsModel(const Tp::AccountManagerPtr &am, QObject *parent)
    : QAbstractItemModel(parent),
      mPriv(new AccountsModel::Private(am))
{
    mPriv->mTree = new TreeNode;
    connect(mPriv->mTree,
            SIGNAL(changed(TreeNode*)),
            SLOT(onItemChanged(TreeNode*)));

    connect(mPriv->mTree,
            SIGNAL(childrenAdded(TreeNode*,QList<TreeNode*>)),
            SLOT(onItemsAdded(TreeNode*,QList<TreeNode*>)));

    connect(mPriv->mTree,
            SIGNAL(childrenRemoved(TreeNode*,int,int)),
            SLOT(onItemsRemoved(TreeNode*,int,int)));

    foreach (Tp::AccountPtr account, mPriv->mAM->allAccounts()) {
        AccountsModelItem *item = new AccountsModelItem(account);
        connect(item, SIGNAL(connectionStatusChanged(QString,int)),
                this, SIGNAL(accountConnectionStatusChanged(QString,int)));
        mPriv->mTree->addChild(item);
    }

    connect(mPriv->mAM.data(),
            SIGNAL(newAccount(Tp::AccountPtr)),
            SLOT(onNewAccount(Tp::AccountPtr)));

    QHash<int, QByteArray> roles;
    roles[ItemRole] = "item";
    roles[IdRole] = "id";
    roles[ValidRole] = "valid";
    roles[EnabledRole] = "enabled";
    roles[ConnectionManagerNameRole] = "connectionManager";
    roles[ProtocolNameRole] = "protocol";
    roles[DisplayNameRole] = "displayName";
    roles[IconRole] = "icon";
    roles[NicknameRole] = "nickname";
    roles[ConnectsAutomaticallyRole] = "connectsAutomatically";
    roles[ChangingPresenceRole] = "changingPresence";
    roles[AutomaticPresenceRole] = "automaticStatus";
    roles[AutomaticPresenceTypeRole] = "automaticStatusType";
    roles[AutomaticPresenceStatusMessageRole] = "automaticStatusMessage";
    roles[CurrentPresenceRole] = "status";
    roles[CurrentPresenceTypeRole] = "statusType";
    roles[CurrentPresenceStatusMessageRole] = "statusMessage";
    roles[RequestedPresenceRole] = "requestedStatus";
    roles[RequestedPresenceTypeRole] = "requestedStatusType";
    roles[RequestedPresenceStatusMessageRole] = "requestedStatusMessage";
    roles[ConnectionStatusRole] = "connectionStatus";
    roles[ConnectionStatusReasonRole] = "connectionStatusReason";
    roles[AliasRole] = "aliasName";
    roles[AvatarRole] = "avatar";
    roles[PresenceStatusRole] = "presenceStatus";
    roles[PresenceTypeRole] = "presenceType";
    roles[PresenceMessageRole] = "presenceMessage";
    roles[SubscriptionStateRole] = "subscriptionState";
    roles[PublishStateRole] = "publishState";
    roles[BlockedRole] = "blocked";
    roles[GroupsRole] = "groups";
    roles[TextChatCapabilityRole] = "textChat";
    roles[MediaCallCapabilityRole] = "mediaCall";
    roles[AudioCallCapabilityRole] = "audioCall";
    roles[VideoCallCapabilityRole] = "videoCall";
    roles[UpgradeCallCapabilityRole] = "upgradeCall";
    roles[FileTransferCapabilityRole] = "fileTransfer";
    setRoleNames(roles);
}

AccountsModel::~AccountsModel()
{
    mPriv->mTree->deleteLater();
    delete mPriv;
}

void AccountsModel::onNewAccount(const Tp::AccountPtr &account)
{
    AccountsModelItem *accountNode = new AccountsModelItem(account);

    connect(accountNode, SIGNAL(connectionStatusChanged(QString,int)),
            this, SIGNAL(accountConnectionStatusChanged(QString,int)));

    onItemsAdded(mPriv->mTree, QList<TreeNode *>() << accountNode);
}

void AccountsModel::onItemChanged(TreeNode *node)
{
    if (node->parent()) {
        //if it is a group item
        if (node->parent() == mPriv->mTree) {
            qobject_cast<AccountsModelItem*>(node)->countOnlineContacts();
        } else {
            qobject_cast<AccountsModelItem*>(node->parent())->countOnlineContacts();
            emit dataChanged(index(node->parent()), index(node->parent()));
        }
    }
    QModelIndex accountIndex = index(node);
    emit dataChanged(accountIndex, accountIndex);
}

void AccountsModel::onItemsAdded(TreeNode *parent, const QList<TreeNode *> &nodes)
{
    QModelIndex parentIndex = index(parent);
    int currentSize = rowCount(parentIndex);
    beginInsertRows(parentIndex, currentSize, currentSize + nodes.size() - 1);
    foreach (TreeNode *node, nodes) {
        parent->addChild(node);
    }
    endInsertRows();
    emit accountCountChanged();
}

void AccountsModel::onItemsRemoved(TreeNode *parent, int first, int last)
{
    QModelIndex parentIndex = index(parent);
    QList<TreeNode *> removedItems;
    beginRemoveRows(parentIndex, first, last);
    for (int i = last; i >= first; i--) {
        parent->childAt(i)->remove();
    }
    endRemoveRows();

    emit accountCountChanged();
}

int AccountsModel::accountCount() const
{
    return mPriv->mTree->size();
}

QObject *AccountsModel::accountItemForId(const QString &id) const
{
    for (int i = 0; i < mPriv->mTree->size(); ++i) {
        AccountsModelItem *item = qobject_cast<AccountsModelItem*>(mPriv->mTree->childAt(i));
        if (!item) {
            continue;
        }

        if (item->data(IdRole) == id) {
            return item;
        }
    }

    return 0;
}

QObject *AccountsModel::contactItemForId(const QString &accountId, const QString &contactId) const
{
    AccountsModelItem *accountItem = qobject_cast<AccountsModelItem*>(accountItemForId(accountId));
    if (!accountItem) {
        return 0;
    }

    for (int i = 0; i < accountItem->size(); ++i) {
        ContactModelItem *item = qobject_cast<ContactModelItem*>(accountItem->childAt(i));
        if (!item) {
            continue;
        }

        if (item->data(IdRole) == contactId) {
            return item;
        }
    }

    return 0;
}

int AccountsModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

int AccountsModel::rowCount(const QModelIndex &parent) const
{
    return mPriv->node(parent)->size();
}

QVariant AccountsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    return mPriv->node(index)->data(role);
}


Tp::AccountPtr AccountsModel::accountForContactItem(ContactModelItem *contactItem) const
{
    AccountsModelItem *accountItem = qobject_cast<AccountsModelItem*>(contactItem->parent());
    if (accountItem) {
        return accountItem->account();
    } else {
        return Tp::AccountPtr();
    }
}

Qt::ItemFlags AccountsModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool AccountsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()) {
        mPriv->node(index)->setData(role, value);
    }

    return false;
}

QModelIndex AccountsModel::index(int row, int column, const QModelIndex &parent) const
{
    TreeNode *parentNode = mPriv->node(parent);
    Q_ASSERT(parentNode);
    if (row < parentNode->size()) {
        return createIndex(row, column, parentNode->childAt(row));
    }

    return QModelIndex();
}

QModelIndex AccountsModel::index(TreeNode *node) const
{
    if (node->parent()) {
        return createIndex(node->parent()->indexOf(node), 0, node);
    } else {
        return QModelIndex();
    }
}

QModelIndex AccountsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    TreeNode *currentNode = mPriv->node(index);
    if (currentNode->parent()) {
        return AccountsModel::index(currentNode->parent());
    } else {
        // no parent: return root node
        return QModelIndex();
    }
}

#include "accounts-model.moc"
