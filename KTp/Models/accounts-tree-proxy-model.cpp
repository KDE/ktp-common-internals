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

#include "accounts-tree-proxy-model.h"

#include "contacts-model.h"

#include <TelepathyQt/Account>

#include <KIcon>

AccountsTreeProxyModel::AccountsTreeProxyModel(QAbstractItemModel *sourceModel, const Tp::AccountManagerPtr &accountManager) :
    AbstractGroupingProxyModel(sourceModel),
    m_accountManager(accountManager)
{

}

QSet<QString> AccountsTreeProxyModel::groupsForIndex(const QModelIndex &sourceIndex) const
{
    const Tp::AccountPtr account = sourceIndex.data(ContactsModel::AccountRole).value<Tp::AccountPtr>();
    if (account) {
        qDebug() << "account";
        return QSet<QString>() << account->objectPath();
    } else {
        qDebug() << "no account";
        return QSet<QString>() << QLatin1String("Unknown");
    }
}


QVariant AccountsTreeProxyModel::dataForGroup(const QString &group, int role) const
{
    Tp::AccountPtr account;
    switch(role) {
    case Qt::DisplayRole:
        account = m_accountManager->accountForObjectPath(group);
        if (account) {
            return account->normalizedName();
        }
        break;
    case ContactsModel::IconRole:
        account = m_accountManager->accountForObjectPath(group);
        if (account) {
            return account->iconName();
        }
        break;
    case ContactsModel::AccountRole:
        return QVariant::fromValue(m_accountManager->accountForObjectPath(group));
    case ContactsModel::TypeRole:
        return ContactsModel::AccountRowType;
    case ContactsModel::EnabledRole:
        account = m_accountManager->accountForObjectPath(group);
        if (account) {
            return account->isEnabled();
        }
        return true;
    case ContactsModel::ConnectionStatusRole:
        account = m_accountManager->accountForObjectPath(group);
        if (account) {
            return account->connectionStatus();
        }
        return Tp::ConnectionStatusConnected;
    }
    return QVariant();
}
