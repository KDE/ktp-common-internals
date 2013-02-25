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

#ifndef ROLES_PROXY_MODEL_H
#define ROLES_PROXY_MODEL_H

#include <QAbstractProxyModel>

class RolesProxyModel : public QAbstractProxyModel {

    Q_OBJECT

public:
    RolesProxyModel(QObject *parent = 0);
    virtual ~RolesProxyModel();

    virtual void setSourceModel(QAbstractItemModel *sourceModel);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;

protected Q_SLOTS:
    virtual void onSourceRowsInserted(const QModelIndex &parent, int start, int end);
    virtual void onSourceRowsRemoved(const QModelIndex &parent, int start, int end);
    virtual void onSourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void onSourceLayoutChanged();

};


#endif

