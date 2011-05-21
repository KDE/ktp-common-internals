/*
 * Contact groups model
 * This file is based on TelepathyQt4Yell Models
 *
 * Copyright (C) 2010 Collabora Ltd. <http://www.collabora.co.uk/>
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

#ifndef TELEPATHY_GROUPS_MODEL_H
#define TELEPATHY_GROUPS_MODEL_H

#include <QAbstractListModel>

#include <TelepathyQt4/Account>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Types>

class ContactModelItem;
class GroupsModelItem;
class AccountsModel;
class ProxyTreeNode;
class TreeNode;

class GroupsModel : public QAbstractItemModel
{
    Q_OBJECT
//    Q_DISABLE_COPY(GroupsModel)const AccountsModel& am, QObject* parentconst AccontsModel& am, QObject* parent
//     Q_PROPERTY(int accountCount READ accountCount NOTIFY accountCountChanged)
    Q_ENUMS(Role)

public:
    enum Role {
        // general roles
        GroupNameRole = Qt::DisplayRole
    };

    explicit GroupsModel(AccountsModel* am, QObject* parent = 0);
    virtual ~GroupsModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(TreeNode *node) const;
    virtual QModelIndex parent(const QModelIndex &index) const;

    void addContactToGroups(ContactModelItem* contactItem, QStringList groups = QStringList());
    ///Convenience classes for addContactToGroups
    void addContactToGroups(ProxyTreeNode* proxyNode, const QString& group);
    void addContactToGroups(ContactModelItem* contactItem, const QString& group);

    void removeContactFromGroup(ProxyTreeNode* proxyNode, const QString& group);

// Q_SIGNALS:
//     void accountCountChanged();
//     void accountConnectionStatusChanged(const QString &accountId, int status);

private Q_SLOTS:
//     void onNewAccount(const Tp::AccountPtr &account);
    void onItemChanged(TreeNode *node);
    void loadAccountsModel();
    void onItemsAdded(TreeNode *parent, const QList<TreeNode *> &nodes);
    void onItemsRemoved(TreeNode *parent, int first, int last);

    void onSourceItemsAdded(TreeNode *parent, const QList<TreeNode *> &nodes);
    void onSourceItemsRemoved(TreeNode *parent, int first, int last);
    void onContactAddedToGroup(const QString &group);
    void onContactRemovedFromGroup(const QString &group);

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // TELEPATHY_GROUPS_MODEL_H
