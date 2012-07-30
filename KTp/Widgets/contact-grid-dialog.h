/*
 * Contact Chooser Dialog
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2012 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
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


#ifndef CONTACT_GRID_DIALOG_H
#define CONTACT_GRID_DIALOG_H

#include <KDialog>
#include <TelepathyQt/Types>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/Contact>

namespace Tp {
class PendingOperation;
}

class AccountsModel;
class QTcpSocket;


namespace KTp {
class ContactGridWidget;

class ContactGridDialog : public KDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactGridDialog)

public:
    ContactGridDialog(QWidget *parent);

    Tp::AccountPtr account();
    Tp::ContactPtr contact();

private Q_SLOTS:
    void onAccountManagerReady();
    void onOkClicked();
    void onChanged();

private:
    Tp::AccountManagerPtr m_accountManager;
    AccountsModel *m_accountsModel;
    KTp::ContactGridWidget *m_contactGridWidget;
    Tp::AccountPtr m_account;
    Tp::ContactPtr m_contact;

};

} // namespace KTp

#endif // CONTACT_GRID_DIALOG_H
