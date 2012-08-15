/*
 * Contact groups model item, represents a group in the contactlist tree
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

#include "groups-model-item.h"

#include <TelepathyQt/Account>
#include <TelepathyQt/ContactManager>

#include <KLocalizedString>

#include "groups-model.h"
#include "accounts-model.h"
#include "proxy-tree-node.h"
#include "contact-model-item.h"

struct GroupsModelItem::Private
{
    Private(const QString &groupName)
        : mGroupName(groupName)
    {
    }

    void setGroupName(const QString &value);
    QString groupName();

    QString mGroupName;
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
    case Qt::DisplayRole:
        if (mPriv->mGroupName.isEmpty()) {
            return i18n("Ungrouped");
        } else {
            return mPriv->mGroupName;
        }
    case AccountsModel::ItemRole:
        return QVariant::fromValue((GroupsModelItem*)this);
    case AccountsModel::IdRole:
        return mPriv->mGroupName.isEmpty() ? QLatin1String("default_group_name") : mPriv->groupName();
    case GroupsModel::GroupNameRole:
        return mPriv->mGroupName;
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
    Q_EMIT childrenAdded(this, QList<TreeNode*>() << proxyNode);

    //the group counters needs to be updated
    Q_EMIT changed(this);
}

void GroupsModelItem::removeProxyContact(ProxyTreeNode *proxyNode)
{
    Q_EMIT childrenRemoved(this, indexOf(proxyNode), indexOf(proxyNode));

    //the group counters needs to be updated
    Q_EMIT changed(this);
}
