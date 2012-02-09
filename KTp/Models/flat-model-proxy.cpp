/*
 * This file is part of TelepathyQt4Yell Models
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.com>
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

#include "flat-model-proxy.h"


struct FlatModelProxy::Private
{
    int offsetOf(const FlatModelProxy *model, int index) const;
};

int FlatModelProxy::Private::offsetOf(const FlatModelProxy *model, int index) const
{
    int offset = 0;
    for (int i = 0; i < index; i++) {
        offset += model->sourceModel()->rowCount(model->sourceModel()->index(i, 0, QModelIndex()));
    }
    return offset;
}

FlatModelProxy::FlatModelProxy(QAbstractItemModel *source)
    : QAbstractProxyModel(source),
      mPriv(new Private())
{
    setSourceModel(source);

    connect(source,
            SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            SLOT(onRowsAboutToBeInserted(QModelIndex,int,int)));
    connect(source,
            SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(onRowsInserted(QModelIndex,int,int)));
    connect(source,
            SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            SLOT(onRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(source,
            SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(onRowsRemoved(QModelIndex,int,int)));
    connect(source,
            SIGNAL(rowsInserted(QModelIndex,int,int)),
            SIGNAL(rowCountChanged()));
    connect(source,
            SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SIGNAL(rowCountChanged()));
    connect(source,
            SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(onDataChanged(QModelIndex,QModelIndex)));
}

FlatModelProxy::~FlatModelProxy()
{
    delete mPriv;
}

QModelIndex FlatModelProxy::mapFromSource(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    QModelIndex parent = index.parent();

    if (!parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(mPriv->offsetOf(this, parent.row()) + index.row(), index.column(), parent.row());
}

QModelIndex FlatModelProxy::mapToSource(const QModelIndex &index) const
{
    int parentRow = index.internalId();
    QModelIndex parent = sourceModel()->index(parentRow, 0, QModelIndex());
    int row = index.row() - mPriv->offsetOf(this, parent.row());
    return sourceModel()->index(row, index.column(), parent);
}

QModelIndex FlatModelProxy::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    int count = 0;
    for (int i = 0; i < sourceModel()->rowCount(QModelIndex()); i++) {
        QModelIndex sourceIndex = sourceModel()->index(i, 0, QModelIndex());
        count += sourceModel()->rowCount(sourceIndex);
        if (row < count) {
            return createIndex(row, column, i);
        }
    }

    return QModelIndex();
}

QModelIndex FlatModelProxy::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int FlatModelProxy::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

int FlatModelProxy::rowCount() const
{
    return rowCount(QModelIndex());
}

int FlatModelProxy::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mPriv->offsetOf(this, sourceModel()->rowCount(QModelIndex()));
}

void FlatModelProxy::onRowsAboutToBeInserted(const QModelIndex &index, int first, int last)
{
    if (index.isValid()) {
        int offset = mPriv->offsetOf(this, index.row());
        int firstIndex = offset + first;
        int lastIndex = offset + last;

        beginInsertRows(QModelIndex(), firstIndex, lastIndex);
    }
}

void FlatModelProxy::onRowsAboutToBeRemoved(const QModelIndex &index, int first, int last)
{
    if (index.isValid()) {
        int offset = mPriv->offsetOf(this, index.row());
        int firstIndex = offset + first;
        int lastIndex = offset + last;

        beginRemoveRows(QModelIndex(), firstIndex, lastIndex);
    }
}

void FlatModelProxy::onRowsInserted(const QModelIndex &index, int first, int last)
{
    Q_UNUSED(first)
    Q_UNUSED(last)
    if (index.isValid()) {
        endInsertRows();
    }
}

void FlatModelProxy::onRowsRemoved(const QModelIndex &index, int first, int last)
{
    Q_UNUSED(first)
    Q_UNUSED(last)
    if (index.isValid()) {
        endRemoveRows();
    }
}

void FlatModelProxy::onDataChanged(const QModelIndex &first, const QModelIndex &last)
{
    if (!first.parent().isValid()) {
        int firstOffset = mPriv->offsetOf(this, first.row());
        int lastOffset = mPriv->offsetOf(this, last.row() + 1) - 1;

        QModelIndex firstIndex = createIndex(firstOffset, 0, first.row());
        QModelIndex lastIndex = createIndex(lastOffset, 0, last.row());
        Q_EMIT dataChanged(firstIndex, lastIndex);
    }
    else if (first.parent() == last.parent()) {
        QModelIndex firstIndex = mapFromSource(first);
        QModelIndex lastIndex = mapFromSource(last);
        Q_EMIT dataChanged(firstIndex, lastIndex);
    }
}
