/*
 * This file is part of telepathy-kde-models-test-ui
 *
 * Copyright (C) 2011 Collabora Ltd. <info@collabora.co.uk>
 * Copyright (C) 2013 David Edmundson <davidedmundson@kde.org>
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

#include "roles-proxy-model.h"

#include <QDebug>

RolesProxyModel::RolesProxyModel(QObject *parent)
  : QAbstractProxyModel(parent)
{
    qDebug();
}

RolesProxyModel::~RolesProxyModel()
{
    qDebug();
}

void RolesProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(onSourceRowsInserted(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(onSourceRowsRemoved(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(onSourceDataChanged(QModelIndex,QModelIndex)));
    connect(sourceModel, SIGNAL(layoutChanged()),
            this, SLOT(onSourceLayoutChanged()));

    QAbstractProxyModel::setSourceModel(sourceModel);
}

int RolesProxyModel::columnCount(const QModelIndex &parent) const
{   
    Q_UNUSED(parent);
    // Number of columns is number of roles available to QML
    return sourceModel()->roleNames().size();
}

int RolesProxyModel::rowCount(const QModelIndex &parent) const
{
    return sourceModel()->rowCount(mapToSource(parent));
}

QModelIndex RolesProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    // this is a list, not a tree, so parent must be invalid (root item)
    if (parent.isValid()) {
        return QModelIndex();
    }

    if (column >= columnCount() || column < 0) {
        return QModelIndex();
    }

    // Check the row is within the bounds of the list
    if (row >= rowCount() || row < 0) {
        return QModelIndex();
    }

    // Return the index to the item.
    return createIndex(row, column);
}

QModelIndex RolesProxyModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);

    // Not a tree model, so all items have root-item as parent.
    return QModelIndex();
}

QVariant RolesProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.column() >= columnCount() || index.column() < 0) {
        return QVariant();
    }

    if (index.row() >= rowCount() || index.row() < 0) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return sourceModel()->data(mapToSource(index), sourceModel()->roleNames().keys().at(index.column()));
    }

    return sourceModel()->data(mapToSource(index), role);
}

Qt::ItemFlags RolesProxyModel::flags(const QModelIndex &index) const
{
    // Pass through from source model
    return sourceModel()->flags(mapToSource(index));
}

QVariant RolesProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    if (role == Qt::DisplayRole) {
        return sourceModel()->roleNames().values().at(section);
    }

    return QVariant();
}

QModelIndex RolesProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }

    return index(sourceIndex.row(), sourceIndex.column(), mapFromSource(sourceIndex.parent()));
}

QModelIndex RolesProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid()) {
        return QModelIndex();
    }

    return sourceModel()->index(proxyIndex.row(), 0, mapToSource(proxyIndex.parent()));
}

void RolesProxyModel::onSourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    beginInsertRows(mapFromSource(parent), start, end);
    endInsertRows();
}

void RolesProxyModel::onSourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    beginRemoveRows(mapFromSource(parent), start, end);
    endRemoveRows();
}

void RolesProxyModel::onSourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // Assume that since the data is layed out in the same way in the proxy model that we can just
    // translate the ranges to relay the signal
    dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight));
}

void RolesProxyModel::onSourceLayoutChanged()
{
    layoutAboutToBeChanged();
    layoutChanged();
}

