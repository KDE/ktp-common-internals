/*
 * This file is part of ktp-common-internals
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.com>
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

#include "accounts-list-model.h"

#include <QIcon>
#include "debug.h"
#include <KLocalizedString>
#include <KPixmapSequence>

#include <KTp/error-dictionary.h>
#include <KTp/presence.h>

#include <TelepathyQt/Account>
#include <TelepathyQt/AccountSet>

class KTp::AccountsListModel::Private {
public:
    QList<Tp::AccountPtr> accounts;
    Tp::AccountSetPtr accountSet;

};


KTp::AccountsListModel::AccountsListModel(QObject *parent)
 : QAbstractListModel(parent),
   d(new AccountsListModel::Private)
{
}

KTp::AccountsListModel::~AccountsListModel()
{
    delete d;
}

QHash<int, QByteArray> KTp::AccountsListModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[ConnectionStateRole] = "connectionState";
    roles[ConnectionStateDisplayRole] = "conectionStateDisplay";
    roles[ConnectionStateIconRole] = "connectionStateIcon";
    roles[ConnectionErrorMessageDisplayRole] = "connectionErrorMessage";
    roles[ConnectionProtocolNameRole] = "connectionProtocolName";
    roles[EnabledRole] = "enabled";
    roles[AccountRole] = "account";
    return roles;
}

void KTp::AccountsListModel::setAccountSet(const Tp::AccountSetPtr &accountSet)
{
    beginResetModel();
    d->accounts.clear();
    endResetModel();

    d->accountSet = accountSet;
    Q_FOREACH(const Tp::AccountPtr &account, d->accountSet->accounts()) {
        onAccountAdded(account);
    }
    connect(d->accountSet.data(), SIGNAL(accountAdded(Tp::AccountPtr)), SLOT(onAccountAdded(Tp::AccountPtr)));
    connect(d->accountSet.data(), SIGNAL(accountRemoved(Tp::AccountPtr)), SLOT(onAccountRemoved(Tp::AccountPtr)));

}

int KTp::AccountsListModel::rowCount(const QModelIndex & parent) const
{
    // If the index is the root item, then return the row count.
    if (parent == QModelIndex()) {
       return d->accounts.size();
    }

    // Otherwise, return 0 (as this is a list model, so all items
    // are children of the root item).
    return 0;
}

int KTp::AccountsListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    // Column count is always 1
    return 1;
}


QVariant KTp::AccountsListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QVariant data;
    Tp::AccountPtr account = d->accounts.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        data = QVariant(account->displayName());
        break;

    case Qt::DecorationRole:
        data = QVariant(QIcon::fromTheme(account->iconName()));
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

    case AccountsListModel::EnabledRole:
        if (account->isEnabled()) {
            data = QVariant(Qt::Checked);
        } else {
            data = QVariant(Qt::Unchecked);
        }
        break;

    case AccountsListModel::AccountRole:
        data = QVariant::fromValue<Tp::AccountPtr>(account);
        break;

    default:
        break;
    }

    return data;
}

bool KTp::AccountsListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    if (role == AccountsListModel::EnabledRole) {
        //this is index from QSortFilterProxyModel
        index.data(AccountRole).value<Tp::AccountPtr>()->setEnabled(value.toInt() == Qt::Checked);
        return true;
    }

    return false;
}

QModelIndex KTp::AccountsListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column < 0 || parent != QModelIndex()) {
        return QModelIndex();
    }

    if (row < rowCount() && column < columnCount()) {
        return createIndex(row, column);
    }

    return QModelIndex();
}

void KTp::AccountsListModel::onAccountAdded(const Tp::AccountPtr &account)
{
    qCDebug(KTP_MODELS) << "Creating a new Account from account:" << account.data();

    // Check if the account is already in the model.
    bool found = false;

    if (!found) {
        Q_FOREACH (const Tp::AccountPtr &ai, d->accounts) {
            if (ai == account) {
                found = true;
                break;
            }
        }
    }

    if (found) {
        qCWarning(KTP_MODELS) << "Requested to add account"
                   << account.data()
                   << "to model, but it is already present. Doing nothing.";
    } else {
        qCDebug(KTP_MODELS) << "Account not already in model. Create new Account from account:"
                 << account.data();

        beginInsertRows(QModelIndex(), d->accounts.size(), d->accounts.size());
        d->accounts.append(account);
        endInsertRows();

        connect(account.data(),
                SIGNAL(stateChanged(bool)),
                SLOT(onAccountUpdated()));
        connect(account.data(),
                SIGNAL(displayNameChanged(QString)),
                SLOT(onAccountUpdated()));
        connect(account.data(),
                SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
                SLOT(onAccountUpdated()));
        connect(account.data(),
                SIGNAL(currentPresenceChanged(Tp::Presence)),
                SLOT(onAccountUpdated()));
        connect(account.data(),
                SIGNAL(iconNameChanged(QString)),
                SLOT(onAccountUpdated()));
        connect(account.data(),
                SIGNAL(stateChanged(bool)),
                SLOT(onAccountUpdated()));
    }
}

void KTp::AccountsListModel::onAccountRemoved(const Tp::AccountPtr &account)
{
    beginRemoveRows(QModelIndex(), d->accounts.indexOf(account), d->accounts.indexOf(account));
    d->accounts.removeAll(account);
    endRemoveRows();
}

void KTp::AccountsListModel::onAccountUpdated()
{
    Tp::AccountPtr item = Tp::AccountPtr(qobject_cast<Tp::Account*>(sender()));

    Q_ASSERT(item);
    if (!item) {
        qCWarning(KTP_MODELS) << "Not an Account pointer:" << sender();
        return;
    }

    QModelIndex index = createIndex(d->accounts.lastIndexOf(item), 0);
    Q_EMIT dataChanged(index, index);
}

const QString KTp::AccountsListModel::connectionStateString(const Tp::AccountPtr &account) const
{
    if (account->isEnabled()) {
        switch (account->connectionStatus()) {
        case Tp::ConnectionStatusConnected:
            return KTp::Presence(account->currentPresence()).displayString();
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

const QIcon KTp::AccountsListModel::connectionStateIcon(const Tp::AccountPtr &account) const
{
    if (account->isEnabled()) {
        switch (account->connectionStatus()) {
        case Tp::ConnectionStatusConnected:
            return KTp::Presence(account->currentPresence()).icon();
        case Tp::ConnectionStatusConnecting:
            //imho this is not really worth animating, but feel free to play around..
            return QIcon(KPixmapSequence(QLatin1String("process-working"), 22).frameAt(0));
        case Tp::ConnectionStatusDisconnected:
            return QIcon::fromTheme(QStringLiteral("user-offline"));
        default:
            return QIcon::fromTheme(QStringLiteral("user-offline"));
        }
    } else {
        return QIcon();
    }
}

const QString KTp::AccountsListModel::connectionStatusReason(const Tp::AccountPtr &account) const
{
    if (account->connectionStatusReason() == Tp::ConnectionStatusReasonRequested) {
        return QString();
    } else {
        return KTp::ErrorDictionary::displayShortErrorMessage(account->connectionError());
    }
}

#include "accounts-list-model.moc"
