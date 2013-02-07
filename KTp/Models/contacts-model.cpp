/*
 * Model of all accounts with inbuilt grouping and filtering
 *
 * Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
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

#include "contacts-model.h"

#include "contacts-list-model.h"
#include "accounts-tree-proxy-model.h"
#include "groups-tree-proxy-model.h"

namespace KTp
{
class ContactsModel::Private
{
public:
    GroupMode groupMode;
    QWeakPointer<KTp::AbstractGroupingProxyModel> proxy;
    KTp::ContactsListModel *source;
    Tp::AccountManagerPtr accountManager;
};
}


KTp::ContactsModel::ContactsModel(QObject *parent)
    : KTp::ContactsFilterModel(parent),
      d(new Private)
{
    d->groupMode = NoGrouping;
    d->source = new KTp::ContactsListModel(this);
}

KTp::ContactsModel::~ContactsModel()
{
    delete d;
}


void KTp::ContactsModel::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    d->accountManager = accountManager;

    updateGroupProxyModels();

    //set the account manager after we've reloaded the groups so that we don't send a list to the view, only to replace it with a grouped tree
    d->source->setAccountManager(accountManager);
}

Tp::AccountManagerPtr KTp::ContactsModel::accountManager() const
{
    return d->accountManager;
}

void KTp::ContactsModel::setGroupMode(KTp::ContactsModel::GroupMode mode)
{
    if (mode == d->groupMode) {
        //if nothing has changed, do nothing.
        return;
    }

    d->groupMode = mode;

    updateGroupProxyModels();

    Q_EMIT groupModeChanged();
}

void KTp::ContactsModel::updateGroupProxyModels()
{
    //if there no account manager there's not a lot point doing anything
    if (!d->accountManager) {
        return;
    }

    //delete any previous proxy
    if (d->proxy) {
        d->proxy.data()->deleteLater();
    }

    switch (d->groupMode) {
    case NoGrouping:
        setSourceModel(d->source);
        break;
    case AccountGrouping:
        d->proxy = new KTp::AccountsTreeProxyModel(d->source, d->accountManager);
        setSourceModel(d->proxy.data());
        break;
    case GroupGrouping:
        d->proxy = new KTp::GroupsTreeProxyModel(d->source);
        setSourceModel(d->proxy.data());
        break;
    }
}

KTp::ContactsModel::GroupMode KTp::ContactsModel::groupMode() const
{
    return d->groupMode;
}
