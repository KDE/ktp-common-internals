/*
 * Turns a list model into a tree allowing nodes to be in multiple groups
 *
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

#include "abstract-grouping-proxy-model.h"

#include <QSet>
#include <QTimer>

#include <KDebug>


class KTp::AbstractGroupingProxyModel::Private
{
public:
    QAbstractItemModel *source;

    //keep a cache of what groups an item belongs to
    QHash<QPersistentModelIndex, QSet<QString> > groupCache;

    //item -> groups
    QMultiHash<QPersistentModelIndex, ProxyNode*> proxyMap;
    QHash<QString, GroupNode*> groupMap;
};

class ProxyNode : public QStandardItem
{
public:
    ProxyNode(const QPersistentModelIndex &sourceIndex);
    QVariant data(int role) const;
    void changed(); //expose protected method in QStandardItem
    QString group() const;
private:
    const QPersistentModelIndex m_sourceIndex;
};

class GroupNode : public QStandardItem {
public:
    GroupNode(const QString &groupId);
    QString group() const;
    virtual QVariant data(int role) const;
    bool forced() const;
    void changed(); //expose protected method in QStandardItem
    void setForced(bool forced);
private:
    const QString m_groupId;
    bool m_forced;
};


ProxyNode::ProxyNode(const QPersistentModelIndex &sourceIndex):
    QStandardItem(),
    m_sourceIndex(sourceIndex)
{
}

QVariant ProxyNode::data(int role) const
{
    return m_sourceIndex.data(role);
}

void ProxyNode::changed()
{
    QStandardItem::emitDataChanged();
}

QString ProxyNode::group() const
{
    //FIXME is this a hack?
    GroupNode *groupNode = static_cast<GroupNode*>(parent());
    if (groupNode) {
        return groupNode->group();
    }
    return QString();
}



GroupNode::GroupNode(const QString &groupId):
    QStandardItem(),
    m_groupId(groupId),
    m_forced(false)
{
}

QString GroupNode::group() const
{
    return m_groupId;
}

QVariant GroupNode::data(int role) const
{
    KTp::AbstractGroupingProxyModel *proxyModel = qobject_cast<KTp::AbstractGroupingProxyModel*>(model());
    Q_ASSERT(proxyModel);
    return proxyModel->dataForGroup(m_groupId, role);
}

void GroupNode::setForced(bool forced)
{
    m_forced = forced;
}

bool GroupNode::forced() const
{
    return m_forced;
}

void GroupNode::changed()
{
    QStandardItem::emitDataChanged();
}


KTp::AbstractGroupingProxyModel::AbstractGroupingProxyModel(QAbstractItemModel *source):
    QStandardItemModel(source),
    d(new KTp::AbstractGroupingProxyModel::Private())
{
    d->source = source;
    setRoleNames(source->roleNames());

    //we have to process all existing rows in the model, but must never call a virtual method from a constructor as it will crash.
    //we use a single shot timer to get round this
    QTimer::singleShot(0, this, SLOT(onLoad()));
}

KTp::AbstractGroupingProxyModel::~AbstractGroupingProxyModel()
{
    delete d;
}


void KTp::AbstractGroupingProxyModel::forceGroup(const QString &group)
{
    GroupNode* groupNode = itemForGroup(group);
    groupNode->setForced(true);
}

void KTp::AbstractGroupingProxyModel::unforceGroup(const QString &group)
{
    GroupNode* groupNode = d->groupMap[group];
    if (!groupNode) {
        return;
    }

    //mark that this group can be removed when it's empty
    groupNode->setForced(false);

    //if group is already empty remove it
    if (groupNode->rowCount() == 0) {
        takeRow(groupNode->row());
        d->groupMap.remove(groupNode->group());
    }
}

void KTp::AbstractGroupingProxyModel::groupChanged(const QString &group)
{
    GroupNode *node = d->groupMap[group];
    if (node) {
        node->changed();
    }
}


/* Called when source items inserts a row
 *
 * For each new row, create a proxyNode.
 * If it's at the top level add it to the relevant group nodes.
 * Otherwise add it to the relevant proxy nodes in our model
 */

void KTp::AbstractGroupingProxyModel::onRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
    //if top level in root model
    if (!sourceParent.isValid()) {
        for (int i = start; i <= end; i++) {
            QModelIndex index = d->source->index(i, 0, sourceParent);
            Q_FOREACH(const QString &group, groupsForIndex(index)) {
                addProxyNode(index, itemForGroup(group));
            }
        }
    } else {
        for (int i = start; i <= end; i++) {
            QModelIndex index = d->source->index(i, 0, sourceParent);
            QHash<QPersistentModelIndex, ProxyNode*>::const_iterator it = d->proxyMap.find(sourceParent);
            while (it != d->proxyMap.end()  && it.key() == sourceParent) {
                addProxyNode(index, it.value());
                it++;
            }
        }
    }
}

#include <KTp/types.h>

void KTp::AbstractGroupingProxyModel::addProxyNode(const QModelIndex &sourceIndex, QStandardItem *parent)
{
    Q_ASSERT(sourceIndex.isValid());
    if (!sourceIndex.isValid()) {
        return;
    }

    ProxyNode *proxyNode = new ProxyNode(sourceIndex);
    d->proxyMap.insertMulti(sourceIndex, proxyNode);
    parent->appendRow(proxyNode);

    //add proxy nodes for all children of this sourceIndex
    for (int i=0; i < d->source->rowCount(sourceIndex); i++) {
        addProxyNode(sourceIndex.child(i,0), proxyNode);
    }

}

void KTp::AbstractGroupingProxyModel::removeProxyNodes(const QModelIndex &sourceIndex, const QList<ProxyNode *> &removedItems)
{
    Q_FOREACH(ProxyNode *proxy, removedItems) {
        QStandardItem *parentItem = proxy->parent();

        //also remove child items of this proxy node from the proxy map
        for (int i = 0 ; i < d->source->rowCount(sourceIndex) ; i++) {
            //we always remove child(0) because we're deleting the first child each time
            removeProxyNodes(sourceIndex.child(i,0), QList<ProxyNode*>() << dynamic_cast<ProxyNode*>(proxy->child(0)));
        }

        parentItem->removeRow(proxy->row());
        d->proxyMap.remove(sourceIndex, proxy);

        //if the parent item to this proxy node is now empty, and is a top level item
        if (parentItem->rowCount() == 0 && parentItem->parent() == 0 ) {
            GroupNode* groupNode = dynamic_cast<GroupNode*>(parentItem);

            //do not delete forced groups
            if (groupNode->forced() == false) {
                takeRow(groupNode->row());
                d->groupMap.remove(groupNode->group());
            }
        }
    }
}

/*
 * Called when a row is remove from the source model model
 * Find all existing proxy models and delete thems
*/
void KTp::AbstractGroupingProxyModel::onRowsRemoved(const QModelIndex &sourceParent, int start, int end)
{
    for (int i = start; i<=end; i++) {
        QPersistentModelIndex index = d->source->index(i, 0, sourceParent);
        QList<ProxyNode *> itemsToRemove;

        QHash<QPersistentModelIndex, ProxyNode*>::const_iterator it = d->proxyMap.find(index);
        while (it != d->proxyMap.end()  && it.key() == index) {
            kDebug() << "removing row" << index.data();
            itemsToRemove.append(it.value());
            ++it;
        }
        d->groupCache.remove(index);
        removeProxyNodes(index, itemsToRemove);
    }
}

/*
 * Called when source model changes data
 * If it's the top level item in the source model detect if the groups have changed, if so update as appropriately
 * Find all proxy nodes, and make dataChanged() get emitted
 */

void KTp::AbstractGroupingProxyModel::onDataChanged(const QModelIndex &sourceTopLeft, const QModelIndex &sourceBottomRight)
{
    for (int i = sourceTopLeft.row(); i <= sourceBottomRight.row(); i++) {
        QPersistentModelIndex index = sourceTopLeft.sibling(i,0);
        if (!index.isValid()) {
            continue;
        }

        //if top level item
        if (!sourceTopLeft.parent().isValid()) {
            //groupsSet has changed...update as appropriate
            QSet<QString> itemGroups = groupsForIndex(d->source->index(i, 0, sourceTopLeft.parent()));
            if (d->groupCache[index] != itemGroups) {
                d->groupCache[index] = itemGroups;

                //loop through existing proxy nodes, and check each one is still valid.
                QHash<QPersistentModelIndex, ProxyNode*>::const_iterator it = d->proxyMap.find(index);
                QList<ProxyNode*> removedItems;
                while (it != d->proxyMap.end() && it.key() == index) {
                    // if proxy's group is still in the item's groups.
                    if (itemGroups.contains(it.value()->group())) {
                        itemGroups.remove(it.value()->group());
                    } else {
                        //remove the proxy item
                        //cache to list and remove once outside the const_iterator
                        removedItems.append(it.value());

                        kDebug() << "removing " << index.data().toString() << " from group " << it.value()->group();
                    }
                    ++it;
                }

                removeProxyNodes(index, removedItems);

                //remaining items in itemGroups are now the new groups
                Q_FOREACH(const QString &group, itemGroups) {
                    GroupNode *groupNode = itemForGroup(group);
                    addProxyNode(index, groupNode);

                    kDebug() << "adding " << index.data().toString() << " to group " << group;
                }
            }
        }

        //mark all proxy nodes as changed
        QHash<QPersistentModelIndex, ProxyNode*>::const_iterator it = d->proxyMap.find(index);
        while (it != d->proxyMap.end() && it.key() == index) {
            it.value()->changed();
            ++it;
        }
    }
}


void KTp::AbstractGroupingProxyModel::onLoad()
{
    if (d->source->rowCount() > 0) {
        onRowsInserted(QModelIndex(), 0, d->source->rowCount()-1);
    }
    connect(d->source, SIGNAL(modelReset()), SLOT(onModelReset()));
    connect(d->source, SIGNAL(rowsInserted(QModelIndex, int,int)), SLOT(onRowsInserted(QModelIndex,int,int)));
    connect(d->source, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(onRowsRemoved(QModelIndex,int,int)));
    connect(d->source, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onDataChanged(QModelIndex,QModelIndex)));
}

/* Called when source model gets reset
 * Delete all local caches/maps and wipe the current QStandardItemModel
 */

void KTp::AbstractGroupingProxyModel::onModelReset()
{
    clear();
    d->groupCache.clear();
    d->proxyMap.clear();
    d->groupMap.clear();
    kDebug() << "reset";

    if (d->source->rowCount() > 0) {
        onRowsInserted(QModelIndex(), 0, d->source->rowCount()-1);
    }
}

GroupNode* KTp::AbstractGroupingProxyModel::itemForGroup(const QString &group)
{
    if (d->groupMap.contains(group)) {
        return d->groupMap[group];
    } else {
        GroupNode* item = new GroupNode(group);
        appendRow(item);
        d->groupMap[group] = item;
        return item;
    }
}
