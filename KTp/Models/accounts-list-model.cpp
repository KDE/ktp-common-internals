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
#include <KLocalizedString>
#include <KPixmapSequence>

#include <KTp/error-dictionary.h>

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
    Tp::AccountPtr account = m_accounts.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        data = QVariant(account->displayName());
        break;

    case Qt::DecorationRole:
        data = QVariant(KIcon(account->iconName()));
        break;

    case Qt::CheckStateRole:
        if (account->isEnabled()) {
            data = QVariant(Qt::Checked);
        } else {
            data = QVariant(Qt::Unchecked);
        }
        break;

    case AccountsListModel::ConnectionStateRole:
        data = QVariant(account->connectionStatus());
        break;

    case AccountsListModel::ConnectionStateDisplayRole:
        data = QVariant(connectionStateString(account));
        break;

    case AccountsListModel::ConnectionStateIconRole:
        data = QVariant(connectionStateIcon(account));
        break;

    case AccountsListModel::ConnectionErrorMessageDisplayRole:
        data = QVariant(connectionStatusReason(account));
        break;

    case AccountsListModel::ConnectionProtocolNameRole:
        data = QVariant(account->protocolName());
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
        Q_FOREACH (const Tp::AccountPtr &ai, m_accounts) {
            if (ai == account) {
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

        beginInsertRows(QModelIndex(), m_accounts.size(), m_accounts.size());
        m_accounts.append(account);
        endInsertRows();

        connect(account.data(), SIGNAL(removed()), SLOT(onAccountItemRemoved()));

        connect(account.data(),
                SIGNAL(stateChanged(bool)),
                SLOT(onAccountItemUpdated()));
        connect(account.data(),
                SIGNAL(displayNameChanged(QString)),
                SLOT(onAccountItemUpdated()));
        connect(account.data(),
                SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
                SLOT(onAccountItemUpdated()));
        connect(account.data(),
                SIGNAL(iconNameChanged(QString)),
                SLOT(onAccountItemUpdated()));
        connect(account.data(),
                SIGNAL(stateChanged(bool)),
                SLOT(onAccountItemUpdated()));
    }
}

void AccountsListModel::onAccountItemRemoved()
{
    Tp::AccountPtr item = Tp::AccountPtr(qobject_cast<Tp::Account*>(sender()));

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
}

void AccountsListModel::onAccountItemUpdated()
{
    Tp::AccountPtr item = Tp::AccountPtr(qobject_cast<Tp::Account*>(sender()));

    Q_ASSERT(item);
    if (!item) {
        kWarning() << "Not an AccountItem pointer:" << sender();
        return;
    }

    QModelIndex index = createIndex(m_accounts.lastIndexOf(item), 0);
    Q_EMIT dataChanged(index, index);
}

const QString AccountsListModel::connectionStateString(const Tp::AccountPtr &account) const
{
    if (account->isEnabled()) {
        switch (account->connectionStatus()) {
        case Tp::ConnectionStatusConnected:
            return i18n("Online");
        case Tp::ConnectionStatusConnecting:
            return i18nc("This is a connection state", "Connecting");
        case Tp::ConnectionStatusDisconnected:
            return i18nc("This is a connection state", "Disconnected");
        default:
            return i18nc("This is an unknown connection state", "Unknown");
        }
    } else {
        return i18nc("This is a disabled account", "Disabled");
    }
}

const KIcon AccountsListModel::connectionStateIcon(const Tp::AccountPtr &account) const
{
    if (account->isEnabled()) {
        switch (account->connectionStatus()) {
        case Tp::ConnectionStatusConnected:
            return KIcon(QLatin1String("user-online"));
        case Tp::ConnectionStatusConnecting:
            //imho this is not really worth animating, but feel free to play around..
            return KIcon(KPixmapSequence(QLatin1String("process-working"), 22).frameAt(0));
        case Tp::ConnectionStatusDisconnected:
            return KIcon(QLatin1String("user-offline"));
        default:
            return KIcon(QLatin1String("user-offline"));
        }
    } else {
        return KIcon();
    }
}

const QString AccountsListModel::connectionStatusReason(const Tp::AccountPtr &account) const
{
    if (!account->isEnabled()) {
        return i18n("Click checkbox to enable");
    }
    else if (account->connectionStatusReason() == Tp::ConnectionStatusReasonRequested) {
        return QString();
    }
    else {
        return KTp::ErrorDictionary::displayShortErrorMessage(account->connectionError());
    }
}

#include "accounts-list-model.moc"
