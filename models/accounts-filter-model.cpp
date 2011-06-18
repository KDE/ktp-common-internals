/*
 * Provide some filters on the account model
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
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

#include "accounts-filter-model.h"

#include "accounts-model.h"
#include "groups-model.h"
#include "groups-model-item.h"
#include "contact-model-item.h"
#include "accounts-model-item.h"


#include <KDebug>

AccountsFilterModel::AccountsFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      m_showOfflineUsers(false),
      m_filterByName(false)
{

}

void AccountsFilterModel::showOfflineUsers(bool showOfflineUsers)
{
    m_showOfflineUsers = showOfflineUsers;
    invalidateFilter();
}

bool AccountsFilterModel::showOfflineUsers() const
{
    return m_showOfflineUsers;
}

bool AccountsFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    int type = index.data(AccountsModel::ItemRole).userType();
    if (type == qMetaTypeId<ContactModelItem*>()) {
        return filterAcceptsContact(index);
    }
    else if (type == qMetaTypeId<AccountsModelItem*>()) {
        return filterAcceptsAccount(index);
    }
    else if (type == qMetaTypeId<GroupsModelItem*>()) {
        return filterAcceptsGroup(index);
    }
    else {
        kDebug() << "Unknown type found in Account Filter";
        return true;
    }
}

bool AccountsFilterModel::filterAcceptsAccount(const QModelIndex &index) const
{
    bool rowAccepted = true;
    //hide disabled accounts
    if (!index.data(AccountsModel::EnabledRole).toBool()) {
        rowAccepted = false;
    }
    //hide
    if (index.data(AccountsModel::ConnectionStatusRole).toUInt()
        != Tp::ConnectionStatusConnected) {
        rowAccepted = false;
    }
    return rowAccepted;
}

bool AccountsFilterModel::filterAcceptsContact(const QModelIndex &index) const
{
    bool rowAccepted = true;
    if (m_filterByName &&
            !index.data(AccountsModel::AliasRole).toString().contains(m_filterString, Qt::CaseInsensitive)) {
        rowAccepted = false;
    }

    //filter offline users out
    if (!m_showOfflineUsers &&
            ((index.data(AccountsModel::PresenceTypeRole).toUInt()
            == Tp::ConnectionPresenceTypeOffline) ||
            (index.data(AccountsModel::PresenceTypeRole).toUInt()
            == Tp::ConnectionPresenceTypeUnknown))) {
        rowAccepted = false;
    }
    return rowAccepted;
}

bool AccountsFilterModel::filterAcceptsGroup(const QModelIndex &index) const
{
    bool acceptRow = true;
    if (!m_showOfflineUsers) {
        if (index.data(AccountsModel::OnlineUsersCountRole).toInt() == 0) {
            acceptRow = false;
        }
    }
    return acceptRow;
}

void AccountsFilterModel::setFilterString(const QString &str)
{
    m_filterString = str;
    m_filterByName = true;
    invalidateFilter();
}

void AccountsFilterModel::clearFilterString()
{
    m_filterString.clear();
    m_filterByName = false;
    invalidateFilter();
}

bool AccountsFilterModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
        uint leftPresence;
        uint rightPresence;

        QString leftDisplayedName = sourceModel()->data(left).toString();
        QString rightDisplayedName = sourceModel()->data(right).toString();

        if (sortRole() == AccountsModel::PresenceTypeRole) {
            leftPresence = sourceModel()->data(left, AccountsModel::PresenceTypeRole).toUInt();
            rightPresence = sourceModel()->data(right, AccountsModel::PresenceTypeRole).toUInt();

            if (leftPresence == rightPresence) {
                return QString::localeAwareCompare(leftDisplayedName, rightDisplayedName) < 0;
            } else {
                if (leftPresence == Tp::ConnectionPresenceTypeAvailable) {
                    return true;
                }
                if (leftPresence == Tp::ConnectionPresenceTypeUnset ||
                        leftPresence == Tp::ConnectionPresenceTypeOffline ||
                        leftPresence == Tp::ConnectionPresenceTypeUnknown ||
                        leftPresence == Tp::ConnectionPresenceTypeError) {
                    return false;
                }

                return leftPresence < rightPresence;
            }
        } else {
            return QString::localeAwareCompare(leftDisplayedName, rightDisplayedName) < 0;
        }
}

void AccountsFilterModel::setSortByPresence(bool enabled)
{
    if (enabled) {
        setSortRole(AccountsModel::PresenceTypeRole);
    } else {
        setSortRole(Qt::DisplayRole);
    }
}

bool AccountsFilterModel::isSortedByPresence() const
{
    return sortRole() == AccountsModel::PresenceTypeRole;
}


#include "accounts-filter-model.moc"
