/*
 * This file is part of telepathy-accounts-kcm
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.com>
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

#include "accounts-list-model.h"

#include "account-item.h"

#include <KDebug>
#include <KIcon>

#include <TelepathyQt/Account>


AccountsListModel::AccountsListModel(QObject *parent)
 : QAbstractListModel(parent)
{
}

AccountsListModel::~AccountsListModel()
{
}

int AccountsListModel::rowCount(const QModelIndex & parent) const
{
    // If the index is the root item, then return the row count.
    if (parent == QModelIndex()) {
       return m_accounts.size();
    }

    // Otherwise, return 0 (as this is a list model, so all items
    // are children of the root item).
    return 0;
}

int AccountsListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    // Column count is always 1
    return 1;
}


QVariant AccountsListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QVariant data;
    Tp::AccountPtr account = m_accounts.at(index.row())->account();

    switch (role) {
    case Qt::DisplayRole:
        data = QVariant(account->displayName());
        break;

    case Qt::DecorationRole:
        data = QVariant(m_accounts.at(index.row())->icon());
        break;

    case Qt::CheckStateRole:
        if (account->isEnabled()) {
            data = QVariant(Qt::Checked);
        } else {
            data = QVariant(Qt::Unchecked);
        }
        break;

    case AccountsListModel::ConnectionStateRole:
        data = QVariant(m_accounts.at(index.row())->connectionStatus());
        break;

    case AccountsListModel::ConnectionStateDisplayRole:
        data = QVariant(m_accounts.at(index.row())->connectionStateString());
        break;

    case AccountsListModel::ConnectionStateIconRole:
        data = QVariant(m_accounts.at(index.row())->connectionStateIcon());
        break;

    case AccountsListModel::ConnectionErrorMessageDisplayRole:
        data = QVariant(m_accounts.at(index.row())->connectionStatusReason());
        break;

    case AccountsListModel::ConnectionProtocolNameRole:
        data = QVariant(m_accounts.at(index.row())->connectionProtocolName());
        break;

    case AccountsListModel::AccountRole:
        data = QVariant::fromValue<Tp::AccountPtr>(account);
        break;

    default:
        break;
    }

    return data;
}

bool AccountsListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    if (role == Qt::CheckStateRole) {
        //this is index from QSortFilterProxyModel
        index.data(AccountRole).value<Tp::AccountPtr>()->setEnabled(value.toInt() == Qt::Checked);
        return true;
    }

    return false;
}

QModelIndex AccountsListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column < 0 || parent != QModelIndex()) {
        return QModelIndex();
    }

    if (row < rowCount() && column < columnCount()) {
        return createIndex(row, column);
    }

    return QModelIndex();
}


Qt::ItemFlags AccountsListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
}

void AccountsListModel::addAccount(const Tp::AccountPtr &account)
{
    kDebug() << "Creating a new AccountItem from account:" << account.data();

    // Check if the account is already in the model.
    bool found = false;

    if (!found) {
        Q_FOREACH (const AccountItem *ai, m_accounts) {
            if (ai->account() == account) {
                found = true;
                break;
            }
        }
    }

    if (found) {
        kWarning() << "Requested to add account"
                   << account.data()
                   << "to model, but it is already present. Doing nothing.";
    } else {
        kDebug() << "Account not already in model. Create new AccountItem from account:"
                 << account.data();

        AccountItem *item = new AccountItem(account, this);

        beginInsertRows(QModelIndex(), m_accounts.size(), m_accounts.size());
        m_accounts.append(item);
        endInsertRows();

        connect(item, SIGNAL(removed()), SLOT(onAccountItemRemoved()));
        connect(item, SIGNAL(updated()), SLOT(onAccountItemUpdated()));
    }
}

void AccountsListModel::onAccountItemRemoved()
{
    AccountItem *item = qobject_cast<AccountItem*>(sender());

    Q_ASSERT(item);
    if (!item) {
        kWarning() << "Not an AccountItem pointer:" << sender();
        return;
    }

    // We can be pretty sure that there is only one reference to a specific AccountItem in the list
    // If we screw up here, the styling delegate will screw up even more
    beginRemoveRows(QModelIndex(), m_accounts.indexOf(item), m_accounts.indexOf(item));
    m_accounts.removeAll(item);
    endRemoveRows();

    // FIXME: Workaround until the KWidgetItemDelegate gets fixed (probably KDE 4.7)
    //reset();
    delete item;
}

void AccountsListModel::onAccountItemUpdated()
{
    AccountItem *item = qobject_cast<AccountItem*>(sender());

    Q_ASSERT(item);
    if (!item) {
        kWarning() << "Not an AccountItem pointer:" << sender();
        return;
    }

    QModelIndex index = createIndex(m_accounts.lastIndexOf(item), 0);
    Q_EMIT dataChanged(index, index);
}

#include "accounts-list-model.moc"
