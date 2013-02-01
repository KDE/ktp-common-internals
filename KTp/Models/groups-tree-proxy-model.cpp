/*
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

#include "groups-tree-proxy-model.h"

#include "types.h"

#include <KLocalizedString>

class KTp::GroupsTreeProxyModel::Private
{
public:
    //nothing here yet, added anyway in case we need private members without breaking the ABI
};


KTp::GroupsTreeProxyModel::GroupsTreeProxyModel(QAbstractItemModel *sourceModel) :
    AbstractGroupingProxyModel(sourceModel),
    d(new KTp::GroupsTreeProxyModel::Private())
{
}

KTp::GroupsTreeProxyModel::~GroupsTreeProxyModel()
{
    delete d;
}

QSet<QString> KTp::GroupsTreeProxyModel::groupsForIndex(const QModelIndex &sourceIndex) const
{
    QStringList groups = sourceIndex.data(KTp::ContactGroupsRole).value<QStringList>();
    if (groups.isEmpty()) {
        groups.append(QLatin1String("_unsorted"));
    }

    return groups.toSet();
}


QVariant KTp::GroupsTreeProxyModel::dataForGroup(const QString &group, int role) const
{
    switch (role) {
    case KTp::RowTypeRole:
        return KTp::GroupRowType;
    case Qt::DisplayRole:
        if (group == QLatin1String("_unsorted")) {
            return i18n("Unsorted");
        } else {
            return group;
        }
    case KTp::IdRole:
        return group;
    }
    return QVariant();
}
