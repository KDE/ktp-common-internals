/*
 * Accounts model item, represents an account in the contactlist tree
 * This file is based on TelepathyQt4Yell Models
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
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

#ifndef TELEPATHY_ACCOUNTS_MODEL_ITEM_H
#define TELEPATHY_ACCOUNTS_MODEL_ITEM_H

#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Types>

#include <QtCore/QVariant> //needed for declare metatype

#include "tree-node.h"

class AccountsModelItem : public TreeNode
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsModelItem)

public:
    AccountsModelItem(const Tp::AccountPtr &account);
    virtual ~AccountsModelItem();

    Q_INVOKABLE virtual QVariant data(int role) const;
    virtual bool setData(int role, const QVariant &value);
    Q_INVOKABLE Tp::AccountPtr account() const;

    void setEnabled(bool value);

    Q_INVOKABLE void setNickname(const QString &value);

    Q_INVOKABLE void setAutomaticPresence(int type, const QString &status, const QString &statusMessage);
    Q_INVOKABLE void setRequestedPresence(int type, const QString &status, const QString &statusMessage);

    void clearContacts();

    void countOnlineContacts();

Q_SIGNALS:
    void connectionStatusChanged(const QString &accountId, int status);

private Q_SLOTS:
    void addKnownContacts();
    void onRemoved();

    void onChanged();

    void onStatusChanged(Tp::ConnectionStatus status);

    void onNewConnection();
    void onConnectionChanged(const Tp::ConnectionPtr &connection);
    void onContactManagerStateChanged(Tp::ContactListState state);
    void onContactsChanged(const Tp::Contacts &added,
                           const Tp::Contacts &removed);

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

Q_DECLARE_METATYPE(AccountsModelItem*);

#endif // TELEPATHY_ACCOUNTS_MODEL_ITEM_H
