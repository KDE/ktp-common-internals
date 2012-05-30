/*
 * Contact groups model
 * This file is based on TelepathyQtYell Models
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.com>
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

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/Contact>
#include <TelepathyQt/PendingReady>

#include "accounts-model-item.h"
#include "groups-model-item.h"
#include "proxy-tree-node.h"
#include "accounts-model.h"
#include "contact-model-item.h"

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KLocalizedString>

struct GroupsModel::Private
{
    Private(AccountsModel *am)
        : mAM(am)
    {
        KGlobal::locale()->insertCatalog(QLatin1String("ktp-common-internals"));

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

    connect(am,
            SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(onSourceAccountAdded(QModelIndex,int,int)));
    if (am->rowCount() > 0) {
        onSourceAccountAdded(QModelIndex(), 0, am->rowCount()-1);
    }

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
    Q_UNUSED(parent);
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
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
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
    if (row >= 0 && row < parentNode->size() && column == 0) {
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
            Q_EMIT dataChanged(index(node->parent()), index(node->parent()));
        }
    }
    Q_EMIT dataChanged(index(node), index(node));
}

void GroupsModel::onItemsAdded(TreeNode *parent, const QList<TreeNode *> &nodes)
{
    QModelIndex parentIndex = index(parent);
    int currentSize = rowCount(parentIndex);
    beginInsertRows(parentIndex, currentSize, currentSize + nodes.size() - 1);
    Q_FOREACH (TreeNode *node, nodes) {
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


void GroupsModel::onSourceContactsAdded(TreeNode *parent, const QList<TreeNode *> &nodes)
{
    kDebug() << "Adding" << nodes.size() << "nodes...";
    QModelIndex parentIndex = index(parent);

    Q_FOREACH (TreeNode *node, nodes) {
        ContactModelItem *contactItem = qobject_cast<ContactModelItem*>(node);
        QStringList groups = contactItem->contact()->groups();
        addContactToGroups(contactItem, groups);
    }
}

void GroupsModel::onSourceContactsRemoved(TreeNode* parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
}

void GroupsModel::onSourceAccountAdded(const QModelIndex &parent, int start, int end)
{
    //only care about top level inclusion.
    if (parent.isValid() ) {
        return;
    }

    for (int i=start; i <= end; i++) {
        AccountsModelItem *accountItem =mPriv->mAM->index(i).data(AccountsModel::ItemRole).value<AccountsModelItem*>();
        if (accountItem) {
            for (int i = 0; i < accountItem->size(); i++) {
                ContactModelItem *contactItem= qobject_cast<ContactModelItem*>(accountItem->childAt(i));
                if (contactItem) {
                    qDebug() << "contact item found";
                    QStringList groups = contactItem->data(AccountsModel::GroupsRole).toStringList();
                    addContactToGroups(contactItem, groups);
                }
            }

            //we need to connect accounts onSourceContactsAdded/onSourceContactsRemoved to watch for changes
            //and process them directly (directly add/remove the nodes)
            connect(accountItem, SIGNAL(childrenAdded(TreeNode*,QList<TreeNode*>)),
                    this, SLOT(onSourceContactsAdded(TreeNode*,QList<TreeNode*>)));

            connect(accountItem, SIGNAL(childrenRemoved(TreeNode*,int,int)),
                    this, SLOT(onSourceContactsRemoved(TreeNode*,int,int)));
        }

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
        groups.append(QString());
    } else {
        checkUngrouped = true;
    }

    groups.removeDuplicates();

    Q_FOREACH (const QString &group, groups) {
        bool groupExists = false;
        GroupsModelItem *groupItem = 0;

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
                if (savedGroupItem->groupName().isEmpty()) {
                    for (int i = 0; i < savedGroupItem->size(); i++) {
                        ProxyTreeNode *tmpNode = qobject_cast<ProxyTreeNode*>(savedGroupItem->childAt(i));
                        if (tmpNode->data(AccountsModel::ItemRole).value<ContactModelItem*>()->contact()->id() == contactItem->contact()->id()) {
                            removeContactFromGroup(tmpNode, QString());
                            if (groupExists) {
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (!groupExists && !groupItem) {
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
