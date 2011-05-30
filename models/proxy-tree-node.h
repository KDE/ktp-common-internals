/*
 * Proxy tree model node
 *
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

#ifndef TELEPATHY_PROXY_TREE_NODE_H
#define TELEPATHY_PROXY_TREE_NODE_H

#include <QObject>
#include <QVariant>
#include "tree-node.h"

class ContactModelItem;
class ProxyTreeNode : public TreeNode
{
    Q_OBJECT
    Q_DISABLE_COPY(ProxyTreeNode)

public:
    ProxyTreeNode(ContactModelItem* sourceNode);

    virtual ~ProxyTreeNode();

    virtual QVariant data(int role) const;
    virtual bool setData(int role, const QVariant &value);

public Q_SLOTS:
    void onSourceNodeRemoved();
    void onSourceChanged();

Q_SIGNALS:
    void contactAddedToGroup(const QString& group);
    void contactRemovedFromGroup(const QString& group);

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // TELEPATHY_PROXY_TREE_NODE_H
