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

#include <TelepathyQt4/Contact>

#include "proxy-tree-node.h"
#include "tree-node.h"
#include "contact-model-item.h"
#include "accounts-model.h"

struct ProxyTreeNode::Private
{
    Private(ContactModelItem *sourceNode) :
        mSource(sourceNode)
    {
    }

    ~Private()
    {
    }

    ContactModelItem *mSource;
};

ProxyTreeNode::ProxyTreeNode(ContactModelItem *sourceNode)
    : mPriv(new Private(sourceNode))
{
    connect(sourceNode->contact().data(),
            SIGNAL(addedToGroup(QString)),
            SIGNAL(contactAddedToGroup(QString)));

    connect(sourceNode->contact().data(),
            SIGNAL(removedFromGroup(QString)),
            SIGNAL(contactRemovedFromGroup(QString)));

    connect(sourceNode, SIGNAL(destroyed(QObject*)),
            this, SLOT(onSourceNodeRemoved()));

    connect(sourceNode,
            SIGNAL(changed(TreeNode*)),
            SLOT(onSourceChanged()));
}

ProxyTreeNode::~ProxyTreeNode()
{
    delete mPriv;
}

QVariant ProxyTreeNode::data(int role) const
{
    if (!mPriv->mSource) {
        return QVariant();
    }
    return mPriv->mSource->data(role);
}

bool ProxyTreeNode::setData(int role, const QVariant &value)
{
    return false;
}

void ProxyTreeNode::onSourceNodeRemoved()
{
    mPriv->mSource = 0;
    int index = parent()->indexOf(this);
    emit childrenRemoved(parent(), index, index);
}

void ProxyTreeNode::onSourceChanged()
{
    emit changed(this);
}
