/*
 * Contact groups model
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

#include "groups-model.h"

#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/PendingReady>

#include "groups-model-item.h"
#include "proxy-tree-node.h"
#include "accounts-model.h"
#include "contact-model-item.h"

#include <KDebug>

struct GroupsModel::Private
{
    Private(AccountsModel *am)
        : mAM(am)
    {
    }

    TreeNode *node(const QModelIndex &index) const;

    AccountsModel *mAM;
    TreeNode *mTree;
};

TreeNode *GroupsModel::Private::node(const QModelIndex &index) const
{
    TreeNode *node = reinterpret_cast<TreeNode *>(index.internalPointer());
    return node ? node : mTree;
}

GroupsModel::GroupsModel(AccountsModel *am, QObject *parent)
    : QAbstractItemModel(parent),
      mPriv(new GroupsModel::Private(am))
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

    loadAccountsModel();
    QHash<int, QByteArray> roles;
    roles[GroupNameRole] = "groupName";
    setRoleNames(roles);
}

GroupsModel::~GroupsModel()
{
    delete mPriv->mTree;
    delete mPriv;
}

int GroupsModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

int GroupsModel::rowCount(const QModelIndex &parent) const
{
    return mPriv->node(parent)->size();
}

QVariant GroupsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    return mPriv->node(index)->data(role);
}

Qt::ItemFlags GroupsModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        bool isGroup = index.data(AccountsModel::ItemRole).userType() == qMetaTypeId<GroupsModelItem*>();
        if (isGroup) {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
        } else {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
        }
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

Qt::DropActions GroupsModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList GroupsModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.telepathy.contact";
    return types;
}

QMimeData* GroupsModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            ContactModelItem *c = data(index, AccountsModel::ItemRole).value<ContactModelItem*>();
            //We put a contact ID and its account ID to the stream, so we can later recreate the contact using AccountsModel
            stream << c->contact().data()->id() << mPriv->mAM->accountForContactItem(c).data()->objectPath();
        }
    }

    mimeData->setData("application/vnd.telepathy.contact", encodedData);
    return mimeData;
}

bool GroupsModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (!data->hasFormat("application/vnd.telepathy.contact")) {
        return false;
    }

    if (column > 0) {
        return false;
    }

    QByteArray encodedData = data->data("application/vnd.telepathy.contact");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<ContactModelItem*> contacts;

    while (!stream.atEnd()) {
        QString contact;
        QString account;

        //get contact and account out of the stream
        stream >> contact >> account;

        Tp::AccountPtr accountPtr = mPriv->mAM->accountPtrForPath(account);

        //casted pointer is checked below, before first use
        contacts.append(qobject_cast<ContactModelItem*>(mPriv->mAM->contactItemForId(accountPtr->uniqueIdentifier(), contact)));
    }

    foreach (ContactModelItem *contact, contacts) {
        Q_ASSERT(contact);
        QString group = parent.data(GroupsModel::GroupNameRole).toString();

        kDebug() << contact->contact().data()->alias() << "added to group" << group;

        if (group != QLatin1String("Ungrouped")) { //FIXME: consider i18n
            //FIXME: Should we connect this somewhere?
            Tp::PendingOperation *op = contact->contact().data()->manager().data()->addContactsToGroup(group,
                                                                                                       QList<Tp::ContactPtr>() << contact->contact());

            connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                    this, SIGNAL(operationFinished(Tp::PendingOperation*)));
        }
    }

    return true;
}

bool GroupsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()) {
        mPriv->node(index)->setData(role, value);
    }

    return false;
}

QModelIndex GroupsModel::index(int row, int column, const QModelIndex &parent) const
{
    TreeNode *parentNode = mPriv->node(parent);
    if (row < parentNode->size()) {
        return createIndex(row, column, parentNode->childAt(row));
    }

    return QModelIndex();
}

QModelIndex GroupsModel::index(TreeNode *node) const
{
    if (node->parent()) {
        return createIndex(node->parent()->indexOf(node), 0, node);
    } else {
        return QModelIndex();
    }
}

QModelIndex GroupsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    TreeNode *currentNode = mPriv->node(index);
    if (currentNode->parent()) {
        return GroupsModel::index(currentNode->parent());
    } else {
        // no parent: return root node
        return QModelIndex();
    }
}

void GroupsModel::onItemChanged(TreeNode* node)
{
    if (node->parent()) {
        //if it is a group item
        if (node->parent() == mPriv->mTree) {
            GroupsModelItem *groupItem = qobject_cast<GroupsModelItem*>(node);
            Q_ASSERT(groupItem);
            groupItem->countOnlineContacts();
        } else {
            GroupsModelItem *groupItem = qobject_cast<GroupsModelItem*>(node->parent());
            Q_ASSERT(groupItem);
            groupItem->countOnlineContacts();
            emit dataChanged(index(node->parent()), index(node->parent()));
        }
    }
    emit dataChanged(index(node), index(node));
}

void GroupsModel::onItemsAdded(TreeNode *parent, const QList<TreeNode *> &nodes)
{
    QModelIndex parentIndex = index(parent);
    int currentSize = rowCount(parentIndex);
    beginInsertRows(parentIndex, currentSize, currentSize + nodes.size() - 1);
    foreach (TreeNode *node, nodes) {
        parent->addChild(node);
    }
    endInsertRows();
}

void GroupsModel::onItemsRemoved(TreeNode *parent, int first, int last)
{
    kDebug();
    QModelIndex parentIndex = index(parent);
    QList<TreeNode *> removedItems;
    beginRemoveRows(parentIndex, first, last);
    for (int i = last; i >= first; i--) {
        parent->childAt(i)->remove();
    }
    endRemoveRows();

    onItemChanged(parent);
}


void GroupsModel::onSourceItemsAdded(TreeNode *parent, const QList<TreeNode *> &nodes)
{
    kDebug() << "Adding" << nodes.size() << "nodes...";
    QModelIndex parentIndex = index(parent);
    int currentSize = rowCount(parentIndex);
    foreach (TreeNode *node, nodes) {
        ContactModelItem *contactItem = qobject_cast<ContactModelItem*>(node);
        QStringList groups = contactItem->contact()->groups();
        addContactToGroups(contactItem, groups);
    }
}

void GroupsModel::onSourceItemsRemoved(TreeNode* parent, int first, int last)
{

}

void GroupsModel::loadAccountsModel()
{
    for (int x = 0; x < mPriv->mAM->rowCount(); x++) {
        QModelIndex parent = mPriv->mAM->index(x, 0);
        for (int i = 0; i < mPriv->mAM->rowCount(parent); i++) {
            if (mPriv->mAM->data(mPriv->mAM->index(i, 0, parent),
                                 AccountsModel::ItemRole).userType() == qMetaTypeId<ContactModelItem*>()) {

                QStringList groups = mPriv->mAM->data(mPriv->mAM->index(i, 0, parent),
                                                      AccountsModel::GroupsRole).toStringList();

                ContactModelItem *contactItem = mPriv->mAM->data(mPriv->mAM->index(i, 0, parent),
                                                                 AccountsModel::ItemRole).value<ContactModelItem*>();

                addContactToGroups(contactItem, groups);
            }
        }

        //we need to connect accounts onItemsAdded/onItemsRemoved to watch for changes
        //and process them directly (directly add/remove the nodes)
        AccountsModelItem *accountItem = mPriv->mAM->data(parent, AccountsModel::ItemRole).value<AccountsModelItem*>();
        connect(accountItem, SIGNAL(childrenAdded(TreeNode*,QList<TreeNode*>)),
                this, SLOT(onSourceItemsAdded(TreeNode*,QList<TreeNode*>)));

        connect(accountItem, SIGNAL(childrenRemoved(TreeNode*,int,int)),
                this, SLOT(onSourceItemsRemoved(TreeNode*,int,int)));

        kDebug() << "Connecting" << accountItem->account()->displayName() << "to groups model";

    }
}

void GroupsModel::onContactAddedToGroup(const QString& group)
{
    addContactToGroups(qobject_cast<ProxyTreeNode*>(sender()), group);
}

void GroupsModel::onContactRemovedFromGroup(const QString& group)
{
    removeContactFromGroup(qobject_cast<ProxyTreeNode*>(sender()), group);
}

void GroupsModel::removeContactFromGroup(ProxyTreeNode* proxyNode, const QString& group)
{
    QStringList contactGroups = proxyNode->data(AccountsModel::ItemRole).value<ContactModelItem*>()->contact()->groups();

    contactGroups.removeOne(group);

    //if the contact really is in that group, remove it
    if (qobject_cast<GroupsModelItem*>(proxyNode->parent())->groupName() == group) {

        disconnect(proxyNode, SIGNAL(contactAddedToGroup(QString)), 0, 0);
        disconnect(proxyNode, SIGNAL(contactRemovedFromGroup(QString)), 0, 0);

        //if the the contact has no groups left, then put it in Ungrouped group
        if (contactGroups.isEmpty()) {
            addContactToGroups(proxyNode->data(AccountsModel::ItemRole).value<ContactModelItem*>(), contactGroups);
        }

        qobject_cast<GroupsModelItem*>(proxyNode->parent())->removeProxyContact(proxyNode);
        ContactModelItem *contactItem = proxyNode->data(AccountsModel::ItemRole).value<ContactModelItem*>();
        Q_ASSERT(contactItem);
        contactItem->contact().data()->manager().data()->removeContactsFromGroup(group, QList<Tp::ContactPtr>() << contactItem->contact());
    }
}

void GroupsModel::addContactToGroups(ProxyTreeNode* proxyNode, const QString& group)
{
    addContactToGroups(proxyNode->data(AccountsModel::ItemRole).value<ContactModelItem*>(), group);
}

void GroupsModel::addContactToGroups(ContactModelItem* contactItem, const QString& group)
{
    addContactToGroups(contactItem, QStringList(group));
}

void GroupsModel::addContactToGroups(ContactModelItem* contactItem, QStringList groups)
{
    //check if the contact is in Ungrouped group, if it is, it needs to be removed from there
    bool checkUngrouped = false;
    //if the contact has no groups, create an 'Ungrouped' group for it
    if (groups.isEmpty()) {
        groups.append("Ungrouped"); //FIXME i18n
    } else {
        checkUngrouped = true;
    }

    groups.removeDuplicates();

    foreach (const QString &group, groups) {
        bool groupExists = false;
        GroupsModelItem *groupItem;

        //check if the group already exists first
        for (int i = 0; i < mPriv->mTree->children().size(); i++) {
            GroupsModelItem *savedGroupItem = qobject_cast<GroupsModelItem*>(mPriv->mTree->childAt(i));
            if (savedGroupItem->groupName() == group) {
                groupExists = true;
                groupItem = savedGroupItem;

                if (!checkUngrouped) {
                    break;
                }
            }
            if (checkUngrouped) {
                if (savedGroupItem->groupName() == "Ungrouped") {
                    for (int i = 0; i < savedGroupItem->size(); i++) {
                        ProxyTreeNode *tmpNode = qobject_cast<ProxyTreeNode*>(savedGroupItem->childAt(i));
                        if (tmpNode->data(AccountsModel::ItemRole).value<ContactModelItem*>()->contact()->id() == contactItem->contact()->id()) {
                            removeContactFromGroup(tmpNode, QString("Ungrouped"));
                            if (groupExists) {
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (!groupExists) {
            groupItem = new GroupsModelItem(group);
            onItemsAdded(mPriv->mTree, QList<TreeNode *>() << groupItem);
        }

        ProxyTreeNode *proxyNode = new ProxyTreeNode(contactItem);
        groupItem->addProxyContact(proxyNode);

        connect(proxyNode, SIGNAL(contactAddedToGroup(QString)),
                this, SLOT(onContactAddedToGroup(QString)));

        connect(proxyNode, SIGNAL(contactRemovedFromGroup(QString)),
                this, SLOT(onContactRemovedFromGroup(QString)));

    }
}
