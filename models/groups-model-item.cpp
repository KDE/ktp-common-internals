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

#include <TelepathyQt4/Account>
#include <TelepathyQt4/ContactManager>

#include "groups-model-item.h"
#include "groups-model.h"
#include "accounts-model.h"
#include "proxy-tree-node.h"
#include "contact-model-item.h"

struct GroupsModelItem::Private
{
    Private(const QString &groupName)
        : mGroupName(groupName),
          mOnlineUsersCount(0)
    {
    }

    void setGroupName(const QString &value);
    QString groupName();

    QString mGroupName;
    int mOnlineUsersCount;
};

void GroupsModelItem::Private::setGroupName(const QString& value)
{
    mGroupName = value;
}

QString GroupsModelItem::Private::groupName()
{
    return mGroupName;
}

GroupsModelItem::GroupsModelItem(const QString &groupName)
    : mPriv(new Private(groupName))
{
}

GroupsModelItem::~GroupsModelItem()
{
    delete mPriv;
}

QVariant GroupsModelItem::data(int role) const
{
    switch (role) {
    case AccountsModel::ItemRole:
        return QVariant::fromValue((GroupsModelItem*)this);
    case GroupsModel::GroupNameRole:
        return mPriv->mGroupName;
    case AccountsModel::TotalUsersCountRole:
        return size();
    case AccountsModel::OnlineUsersCountRole:
        return mPriv->mOnlineUsersCount;
    default:
        return QVariant();
    }
}

bool GroupsModelItem::setData(int role, const QVariant &value)
{
    switch (role) {
    case GroupsModel::GroupNameRole:
        setGroupName(value.toString());
        return true;
    default:
        return false;
    }
}

void GroupsModelItem::setGroupName(const QString& value)
{
    mPriv->setGroupName(value);
}

QString GroupsModelItem::groupName()
{
    return mPriv->groupName();
}

void GroupsModelItem::addProxyContact(ProxyTreeNode *proxyNode)
{
    emit childrenAdded(this, QList<TreeNode*>() << proxyNode);

    //the group counters needs to be updated
    emit changed(this);
}

void GroupsModelItem::removeProxyContact(ProxyTreeNode *proxyNode)
{
    emit childrenRemoved(this, indexOf(proxyNode), indexOf(proxyNode));

    //the group counters needs to be updated
    emit changed(this);
}

void GroupsModelItem::countOnlineContacts()
{
    int tmpCounter = 0;
    for (int i = 0; i < size(); ++i) {
        ProxyTreeNode* proxyNode = qobject_cast<ProxyTreeNode*>(childAt(i));
        Q_ASSERT(proxyNode);
        if (proxyNode->data(AccountsModel::PresenceTypeRole).toUInt() != Tp::ConnectionPresenceTypeOffline
            && proxyNode->data(AccountsModel::PresenceTypeRole).toUInt() != Tp::ConnectionPresenceTypeUnknown) {
            tmpCounter++;
        }
    }

    mPriv->mOnlineUsersCount = tmpCounter;
}
