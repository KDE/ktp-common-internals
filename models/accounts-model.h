/*
 * Accounts and contacts model
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

#ifndef TELEPATHY_ACCOUNTS_MODEL_H
#define TELEPATHY_ACCOUNTS_MODEL_H

#include <QAbstractListModel>

#include <TelepathyQt4/Account>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Types>

#include "accounts-model-item.h"

class ContactModelItem;

class AccountsModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsModel)
    Q_PROPERTY(int accountCount READ accountCount NOTIFY accountCountChanged)
    Q_ENUMS(Role)

public:
    enum Role {
        // general roles
        ItemRole = Qt::UserRole,
        AvatarRole,
        IdRole,

        // account roles
        ValidRole,
        EnabledRole,
        ConnectionManagerNameRole,
        ProtocolNameRole,
        DisplayNameRole,
        IconRole,
        NicknameRole,
        ConnectsAutomaticallyRole,
        ChangingPresenceRole,
        AutomaticPresenceRole,
        AutomaticPresenceTypeRole,
        AutomaticPresenceStatusMessageRole,
        CurrentPresenceRole,
        CurrentPresenceTypeRole,
        CurrentPresenceStatusMessageRole,
        RequestedPresenceRole,
        RequestedPresenceTypeRole,
        RequestedPresenceStatusMessageRole,
        ConnectionStatusRole,
        ConnectionStatusReasonRole,

        // contact roles
        AliasRole,
        PresenceStatusRole,
        PresenceTypeRole,
        PresenceMessageRole,
        SubscriptionStateRole,
        PublishStateRole,
        BlockedRole,
        GroupsRole,
        TextChatCapabilityRole,
        MediaCallCapabilityRole,
        AudioCallCapabilityRole,
        VideoCallCapabilityRole,
        UpgradeCallCapabilityRole,
        FileTransferCapabilityRole,

        TotalUsersCountRole,
        OnlineUsersCountRole,

        CustomRole // a placemark for custom roles in inherited models
    };

    explicit AccountsModel(const Tp::AccountManagerPtr &am, QObject *parent = 0);
    virtual ~AccountsModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    Tp::AccountPtr accountForContactItem(ContactModelItem* contactItem) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(TreeNode *node) const;
    virtual QModelIndex parent(const QModelIndex &index) const;

    int accountCount() const;
    Q_INVOKABLE QObject *accountItemForId(const QString &id) const;
    Q_INVOKABLE QObject *contactItemForId(const QString &accountId, const QString &contactId) const;

Q_SIGNALS:
    void accountCountChanged();
    void accountConnectionStatusChanged(const QString &accountId, int status);

public Q_SLOTS:
    void onNewAccount(const Tp::AccountPtr &account);
    void onItemChanged(TreeNode *node);
    void onItemsAdded(TreeNode *parent, const QList<TreeNode *> &nodes);
    void onItemsRemoved(TreeNode *parent, int first, int last);

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // TELEPATHY_ACCOUNTS_MODEL_H
