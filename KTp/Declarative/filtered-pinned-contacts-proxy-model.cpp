/*
    Copyright (C) 2012 Aleix Pol <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filtered-pinned-contacts-proxy-model.h"
#include "pinned-contacts-model.h"

FilteredPinnedContactsProxyModel::FilteredPinnedContactsProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

bool FilteredPinnedContactsProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    bool ret = idx.data(PinnedContactsModel::AvailabilityRole).toBool()
           && !idx.data(PinnedContactsModel::AlreadyChattingRole).toBool();
    return ret;
}

