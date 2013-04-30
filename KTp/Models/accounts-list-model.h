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

#ifndef KTP_ACCOUNTS_LIST_MODEL_H
#define KTP_ACCOUNTS_LIST_MODEL_H

#include <QtCore/QAbstractListModel>

#include <TelepathyQt/Account>

#include <KTp/ktp-export.h>
#include <KTp/types.h>

class KIcon;

namespace KTp
{

class KTP_EXPORT AccountsListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsListModel);

public:
    enum Roles {
        ConnectionStateRole = Qt::UserRole,
        ConnectionStateDisplayRole = Qt::UserRole+1,
        ConnectionStateIconRole,
        ConnectionErrorMessageDisplayRole,
        ConnectionProtocolNameRole,
        EnabledRole,
        AccountRole
    };

    explicit AccountsListModel(QObject *parent = 0);
    virtual ~AccountsListModel();

    void setAccountSet(const Tp::AccountSetPtr &accountSet);

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

private Q_SLOTS:
    void onAccountAdded(const Tp::AccountPtr &account);
    void onAccountRemoved(const Tp::AccountPtr &account);
    void onAccountUpdated();

private:
    class Private;
    Private * const d;

    const QString connectionStateString(const Tp::AccountPtr &account) const;
    const KIcon connectionStateIcon(const Tp::AccountPtr &account) const;
    const QString connectionStatusReason(const Tp::AccountPtr &account) const;
};

}

#endif // header guard

