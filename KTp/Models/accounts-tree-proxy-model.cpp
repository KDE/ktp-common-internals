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
#include <TelepathyQt/AccountManager>

#include <KIcon>


class KTp::AccountsTreeProxyModel::Private
{
public:
    Tp::AccountManagerPtr accountManager;
};

KTp::AccountsTreeProxyModel::AccountsTreeProxyModel(QAbstractItemModel *sourceModel, const Tp::AccountManagerPtr &accountManager) :
    AbstractGroupingProxyModel(sourceModel),
    d(new Private())
{
    d->accountManager = accountManager;
}

QSet<QString> KTp::AccountsTreeProxyModel::groupsForIndex(const QModelIndex &sourceIndex) const
{
    const Tp::AccountPtr account = sourceIndex.data(ContactsModel::AccountRole).value<Tp::AccountPtr>();
    if (account) {
        return QSet<QString>() << account->objectPath();
    } else {
        return QSet<QString>() << QLatin1String("Unknown");
    }
}


QVariant KTp::AccountsTreeProxyModel::dataForGroup(const QString &group, int role) const
{
    Tp::AccountPtr account;
    switch(role) {
    case Qt::DisplayRole:
        account = d->accountManager->accountForObjectPath(group);
        if (account) {
            return account->normalizedName();
        }
        break;
    case ContactsModel::IconRole:
        account = d->accountManager->accountForObjectPath(group);
        if (account) {
            return account->iconName();
        }
        break;
    case ContactsModel::AccountRole:
        return QVariant::fromValue(d->accountManager->accountForObjectPath(group));
    case ContactsModel::TypeRole:
        return ContactsModel::AccountRowType;
    case ContactsModel::EnabledRole:
        account = d->accountManager->accountForObjectPath(group);
        if (account) {
            return account->isEnabled();
        }
        return true;
    case ContactsModel::ConnectionStatusRole:
        account = d->accountManager->accountForObjectPath(group);
        if (account) {
            return account->connectionStatus();
        }
        return Tp::ConnectionStatusConnected;
    }

    return QVariant();
}
