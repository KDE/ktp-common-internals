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

#include "contacts-model.h"

#include <KLocalizedString>

GroupsTreeProxyModel::GroupsTreeProxyModel(QAbstractItemModel *sourceModel) :
    AbstractGroupingProxyModel(sourceModel)
{
}

QSet<QString> GroupsTreeProxyModel::groupsForIndex(const QModelIndex &sourceIndex) const
{
    QStringList groups = sourceIndex.data(ContactsModel::GroupsRole).value<QStringList>();
    if (groups.isEmpty()) {
        groups.append(QLatin1String("_unsorted"));
    }

    return groups.toSet();
}


QVariant GroupsTreeProxyModel::dataForGroup(const QString &group, int role) const
{
    switch (role) {
    case ContactsModel::TypeRole:
        return ContactsModel::GroupRowType;
    case Qt::DisplayRole:
        if (group == QLatin1String("_unsorted")) {
            return i18n("Unsorted");
        } else {
            return group;
        }
    case ContactsModel::IdRole:
        return group;
    }
    return QVariant();
}
