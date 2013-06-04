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

#include <KTp/ktp-export.h>

namespace KTp {

class KTP_EXPORT AccountsComboBox : public QComboBox
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsComboBox)
public:
    explicit AccountsComboBox(QWidget *parent = 0);
    virtual ~AccountsComboBox();

public:
    /** A set of accounts to show.
     * See AccountsListModel::setAccountSet for details
     */
    void setAccountSet(const Tp::AccountSetPtr &accountSet);

    /** The currently selected account
     * @return the select account this pointer will be null if no account is selected
     */
    Tp::AccountPtr currentAccount();


    /** Sets the index to the account with the specified ID, provided it exists in the account set
     *  This may be used to save/restore the last used account when showing the combo box
     * @param selectedAccountId the account id as found from Tp::Account::uniqueIdentifier
     */
    void setCurrentAccount(const QString &selectedAccountId);

    /** Sets the index to the specified account, provided it exists in the account set
        @param selectedAccount the account to select
     */
    void setCurrentAccount(const Tp::AccountPtr &selectedAccount);

private:
    class Private;
    Private * const d;
};
}

#endif // ACCOUNTSCOMBOBOX_H
