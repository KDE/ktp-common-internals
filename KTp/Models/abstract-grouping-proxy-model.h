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


#ifndef ABSTRACTGROUPINGPROXYMODEL_H
#define ABSTRACTGROUPINGPROXYMODEL_H

#include <QStandardItemModel>

class ProxyNode;
class GroupNode;

class AbstractGroupingProxyModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit AbstractGroupingProxyModel(QAbstractItemModel *source);

//protected:
    /** Return a list of all groups this items belongs to. Subclasses must override this*/
    virtual QSet<QString> groupsForIndex(const QModelIndex &sourceIndex) const = 0;
    /** Equivalent of QAbstractItemModel::data() called for a specific group header*/
    virtual QVariant dataForGroup(const QString &group, int role) const = 0;

private slots:
    void onRowsInserted(const QModelIndex &sourceParent, int start, int end);
    void onRowsRemoved(const QModelIndex &sourceParent, int start, int end);
    void onDataChanged(const QModelIndex &sourceTopLeft, const QModelIndex &sourceBottomRight);
    void onModelReset();

private:
    /** Create a new proxyNode appended to the given parent in this model*/
    void addProxyNode(const QModelIndex &sourceIndex, QStandardItem *parent);


    /** Returns the standard Item belonging to a particular group name. Creating one if needed*/
    QStandardItem *itemForGroup(const QString &group);

    QAbstractItemModel *m_source;

    //keep a cache of what groups an item belongs to
    QHash<QPersistentModelIndex, QSet<QString> > m_groupCache;

    //item -> groups
    QMultiHash<QPersistentModelIndex, ProxyNode*> m_proxyMap;
    QHash<QString, QStandardItem*> m_groupMap;
};

#endif // GROUPPROXYMODEL_H
