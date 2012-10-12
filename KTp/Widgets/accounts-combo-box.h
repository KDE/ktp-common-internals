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


#ifndef ACCOUNTSCOMBOBOX_H
#define ACCOUNTSCOMBOBOX_H

#include <QComboBox>
#include <TelepathyQt/Types>

namespace KTp {

class AccountsComboBox : public QComboBox
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsComboBox)
public:
    explicit AccountsComboBox(QWidget *parent = 0);

public:
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);
    Tp::AccountPtr currentAccount();

private:
    class Private;
    Private * const d;
};
}

#endif // ACCOUNTSCOMBOBOX_H
