/*
 * Add contact dialog
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
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

#ifndef ADDCONTACTDIALOG_H
#define ADDCONTACTDIALOG_H

#include <KDialog>

#include <TelepathyQt/Types>

#include <KTp/ktp-export.h>

namespace Ui {
    class AddContactDialog;
}

class AccountsModel;

namespace KTp
{
class KTP_EXPORT AddContactDialog : public KDialog
{
    Q_OBJECT

public:
    explicit AddContactDialog(AccountsModel* accountModel, QWidget *parent = 0);
    virtual ~AddContactDialog();
    Tp::AccountPtr account() const;
    const QString screenName() const;

private:
    Ui::AddContactDialog *ui;
};
}

#endif // ADDCONTACTDIALOG_H
