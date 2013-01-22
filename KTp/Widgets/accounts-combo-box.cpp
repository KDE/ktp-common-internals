/*
 * Copyright (C) 2012 David Edmundson <kde@davidedmundson.co.uk>
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


#include "accounts-combo-box.h"

#include <TelepathyQt/AccountSet>
#include <TelepathyQt/Account>

#include <KTp/Models/accounts-list-model.h>


class KTp::AccountsComboBox::Private
{
public:
    AccountsListModel *model;
};

KTp::AccountsComboBox::AccountsComboBox(QWidget *parent) :
    QComboBox(parent),
    d(new KTp::AccountsComboBox::Private())
{
    d->model = new AccountsListModel(this);
    setModel(d->model);
}

void KTp::AccountsComboBox::setAccountSet(const Tp::AccountSetPtr &accountSet)
{
    d->model->setAccountSet(accountSet);
}

Tp::AccountPtr KTp::AccountsComboBox::currentAccount()
{
    return itemData(currentIndex(), AccountsListModel::AccountRole).value<Tp::AccountPtr>();
}

void KTp::AccountsComboBox::setCurrentAccount(const QString &selectedAccountId)
{
    for (int i=0;i<count();i++) {
        if (itemData(i, AccountsListModel::AccountRole).value<Tp::AccountPtr>()->uniqueIdentifier() == selectedAccountId) {
            setCurrentIndex(i);
            break;
        }
    }
}

void KTp::AccountsComboBox::setCurrentAccount(const Tp::AccountPtr &selectedAccount)
{
    setCurrentAccount(selectedAccount->uniqueIdentifier());
}



