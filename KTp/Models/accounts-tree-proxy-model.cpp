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

#include "types.h"

#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountSet>

#include <KIcon>


class KTp::AccountsTreeProxyModel::Private
{
public:
    Tp::AccountManagerPtr accountManager;
    Tp::AccountSetPtr accountSet;
};

KTp::AccountsTreeProxyModel::AccountsTreeProxyModel(QAbstractItemModel *sourceModel, const Tp::AccountManagerPtr &accountManager) :
    AbstractGroupingProxyModel(sourceModel),
    d(new Private())
{
    d->accountManager = accountManager;

    d->accountSet = accountManager->enabledAccounts();
    connect(d->accountSet.data(), SIGNAL(accountAdded(Tp::AccountPtr)), SLOT(onAccountAdded(Tp::AccountPtr)));
    connect(d->accountSet.data(), SIGNAL(accountRemoved(Tp::AccountPtr)), SLOT(onAccountRemoved(Tp::AccountPtr)));
    Q_FOREACH(const Tp::AccountPtr &account, d->accountSet->accounts()) {
        onAccountAdded(account);
    }
}

KTp::AccountsTreeProxyModel::~AccountsTreeProxyModel()
{
    delete d;
}

QSet<QString> KTp::AccountsTreeProxyModel::groupsForIndex(const QModelIndex &sourceIndex) const
{
    const Tp::AccountPtr account = sourceIndex.data(KTp::AccountRole).value<Tp::AccountPtr>();
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
            return account->displayName();
        }
        break;
    case Qt::DecorationRole:
        account = d->accountManager->accountForObjectPath(group);
        if (account) {
            return KIcon(account->iconName());
        }
        break;
    case KTp::AccountRole:
        return QVariant::fromValue(d->accountManager->accountForObjectPath(group));
    case KTp::RowTypeRole:
        return KTp::AccountRowType;
    case KTp::IdRole:
        account = d->accountManager->accountForObjectPath(group);
        if (account) {
            return account->uniqueIdentifier();
        }
        break;
    }

    return QVariant();
}

void KTp::AccountsTreeProxyModel::onAccountChanged()
{
    Tp::AccountPtr account(qobject_cast<Tp::Account*>(sender()));
    groupChanged(account->objectPath());
}

void KTp::AccountsTreeProxyModel::onAccountAdded(const Tp::AccountPtr &account)
{
    forceGroup(account->objectPath());
    connect(account.data(), SIGNAL(normalizedNameChanged(QString)), SLOT(onAccountChanged()));
    connect(account.data(), SIGNAL(iconNameChanged(QString)), SLOT(onAccountChanged()));
}

void KTp::AccountsTreeProxyModel::onAccountRemoved(const Tp::AccountPtr &account)
{
    unforceGroup(account->objectPath());
}
