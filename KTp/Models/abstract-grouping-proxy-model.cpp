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

#include <KDebug>


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
private:
    const QString m_groupId;
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
    m_groupId(groupId)
{
}

QString GroupNode::group() const {
    return m_groupId;
}

QVariant GroupNode::data(int role) const
{
    AbstractGroupingProxyModel *proxyModel = qobject_cast<AbstractGroupingProxyModel*>(model());
    Q_ASSERT(proxyModel);
    return proxyModel->dataForGroup(m_groupId, role);
}


AbstractGroupingProxyModel::AbstractGroupingProxyModel(QAbstractItemModel *source):
    QStandardItemModel(source),
    m_source(source)
{
    if (source->rowCount() > 0) {
        onRowsInserted(QModelIndex(), 0, source->rowCount());
    }

    connect(m_source, SIGNAL(modelReset()), SLOT(onModelReset()));
    connect(m_source, SIGNAL(rowsInserted(QModelIndex, int,int)), SLOT(onRowsInserted(QModelIndex,int,int)));
    connect(m_source, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(onRowsRemoved(QModelIndex,int,int)));
    connect(m_source, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onDataChanged(QModelIndex,QModelIndex)));
}

/* Called when source items inserts a row
 *
 * For each new row, create a proxyNode.
 * If it's at the top level add it to the relevant group nodes.
 * Otherwise add it to the relevant proxy nodes in our model
 */

void AbstractGroupingProxyModel::onRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
    //if top level in root model
    if (!sourceParent.parent().isValid()) {
        for (int i = start; i<=end; i++) {
            QModelIndex index = m_source->index(i, 0, sourceParent);
            foreach(const QString &group, groupsForIndex(index)) {
                addProxyNode(index, itemForGroup(group));
            }
        }
    } else {
        for (int i = start; i<=end; i++) {
            QModelIndex index = m_source->index(i, 0, sourceParent);
            QHash<QPersistentModelIndex, ProxyNode*>::const_iterator it = m_proxyMap.find(index);
            while (it != m_proxyMap.end()  && it.key() == index) {
                addProxyNode(index, it.value());
                it++;
            }
        }
    }
}

void AbstractGroupingProxyModel::addProxyNode(const QModelIndex &sourceIndex, QStandardItem *parent)
{
    ProxyNode *proxyNode = new ProxyNode(sourceIndex);
    m_proxyMap.insertMulti(sourceIndex, proxyNode);
    parent->appendRow(proxyNode);
}

/*
 * Called when a row is remove from the source model model
 * Find all existing proxy models and delete thems
*/
void AbstractGroupingProxyModel::onRowsRemoved(const QModelIndex &sourceParent, int start, int end)
{
    for (int i = start; i<=end; i++) {
        QPersistentModelIndex index = m_source->index(i, 0, sourceParent);

        QHash<QPersistentModelIndex, ProxyNode*>::const_iterator it = m_proxyMap.find(index);
        while (it != m_proxyMap.end()  && it.key() == index) {
            kDebug() << "removing row" << index.data();
            it.value()->parent()->removeRow(it.value()->row());
            ++it;
        }
        m_proxyMap.remove(index);
    }
}

/*
 * Called when source model changes data
 * If it's the top level item in the source model detect if the groups have changed, if so update as appropriately
 * Find all proxy nodes, and make dataChanged() get emitted
 */

void AbstractGroupingProxyModel::onDataChanged(const QModelIndex &sourceTopLeft, const QModelIndex &sourceBottomRight)
{
    for (int i=sourceTopLeft.row() ; i<=sourceBottomRight.row() ; i++) {
        QPersistentModelIndex index = m_source->index(i, 0, sourceTopLeft.parent());

        //if top level item
        if (!sourceTopLeft.parent().isValid()) {
            //groupsSet has changed...update as appropriate
            QSet<QString> itemGroups = groupsForIndex(m_source->index(i, 0, sourceTopLeft.parent()));
            if (m_groupCache[index] != itemGroups) {
                m_groupCache[index] = itemGroups;

                //loop through existing proxy nodes, and check each one is still valid.
                QHash<QPersistentModelIndex, ProxyNode*>::const_iterator it = m_proxyMap.find(index);
                QList<ProxyNode*> removedItems;
                while (it != m_proxyMap.end()  && it.key() == index) {
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

                //do the actual removing
                foreach (ProxyNode *proxy, removedItems) {
                    proxy->parent()->removeRow(proxy->row());
                    m_proxyMap.remove(index, proxy);
                }

                //remaining items in itemGroups are now the new groups
                foreach(const QString &group, itemGroups) {
                    ProxyNode *proxyNode = new ProxyNode(index);
                    m_proxyMap.insertMulti(index, proxyNode);
                    itemForGroup(group)->appendRow(proxyNode);

                    kDebug() << "adding " << index.data().toString() << " to group " << group;
                }
            }
        }

        //mark all proxy nodes as changed
        QHash<QPersistentModelIndex, ProxyNode*>::const_iterator it = m_proxyMap.find(index);
        while (it != m_proxyMap.end() && it.key() == index) {
            it.value()->changed();
            ++it;
        }
    }
}

/* Called when source model gets reset
 * Delete all local caches/maps and wipe the current QStandardItemModel
 */

void AbstractGroupingProxyModel::onModelReset()
{
    clear();
    m_groupCache.clear();
    m_proxyMap.clear();
    m_groupMap.clear();
    kDebug() << "reset";
}

QStandardItem *AbstractGroupingProxyModel::itemForGroup(const QString &group)
{
    if (m_groupMap.contains(group)) {
        return m_groupMap[group];
    } else {
        GroupNode* item = new GroupNode(group);
        appendRow(item);
        m_groupMap[group] = item;
        return item;
    }
}
