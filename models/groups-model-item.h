/*
 * Contact groups model item, represents a group in the contactlist tree
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

#ifndef TELEPATHY_GROUPS_MODEL_ITEM_H
#define TELEPATHY_GROUPS_MODEL_ITEM_H

#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Types>

#include <QtCore/QVariant> //needed for declare metatype

#include "tree-node.h"

class ContactModelItem;
class ProxyTreeNode;

class GroupsModelItem : public TreeNode
{
    Q_OBJECT
    Q_DISABLE_COPY(GroupsModelItem)

public:
    GroupsModelItem(const QString &groupName);
    virtual ~GroupsModelItem();

    Q_INVOKABLE virtual QVariant data(int role) const;
    virtual bool setData(int role, const QVariant &value);

    Q_INVOKABLE void setGroupName(const QString &value);
    QString groupName();

    void addProxyContact(ProxyTreeNode* proxyNode);
    void removeProxyContact(ProxyTreeNode* proxyNode);

    void countOnlineContacts();

private Q_SLOTS:


private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

Q_DECLARE_METATYPE(GroupsModelItem*);

#endif // TELEPATHY_GROUPS_MODEL_ITEM_H
