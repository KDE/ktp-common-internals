/*
 * Contact groups model
 * This file is based on TelepathyQtYell Models
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.com>
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

#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Types>

#include <KTp/ktp-export.h>

class ContactModelItem;
class GroupsModelItem;
class AccountsModel;
class ProxyTreeNode;
class TreeNode;

class KTP_EXPORT GroupsModel : public QAbstractItemModel
{
    Q_OBJECT
//    Q_DISABLE_COPY(GroupsModel)const AccountsModel& am, QObject* parentconst AccontsModel& am, QObject* parent
//     Q_PROPERTY(int accountCount READ accountCount NOTIFY accountCountChanged)
    Q_ENUMS(Role)

public:
    enum Role {
        // general roles
        GroupNameRole = Qt::UserRole + 1000
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

Q_SIGNALS:
    //a signal for reemitting the operation status, used for displaying errors in GUI
    void operationFinished(Tp::PendingOperation *op);

private Q_SLOTS:
//     void onNewAccount(const Tp::AccountPtr &account);
    void onItemChanged(TreeNode *node);
    void onSourceAccountAdded(const QModelIndex &parent, int start, int end);
    void onItemsAdded(TreeNode *parent, const QList<TreeNode *> &nodes);
    void onItemsRemoved(TreeNode *parent, int first, int last);

    void onSourceContactsAdded(TreeNode *parent, const QList<TreeNode *> &nodes);
    void onSourceContactsRemoved(TreeNode *parent, int first, int last);
    void onContactAddedToGroup(const QString &group);
    void onContactRemovedFromGroup(const QString &group);

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // TELEPATHY_GROUPS_MODEL_H
