/*
 * This file is part of telepathy-accounts-kcm
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

#ifndef WALLETINTERFACE_H
#define WALLETINTERFACE_H

#include <TelepathyQt4/Account>

#include <KWallet/Wallet>

#include <QScopedPointer>

class WalletInterface
{
public:

    WalletInterface(WId winId);
    virtual ~WalletInterface();

    /** Returns true if a password is stored for this acount*/
    bool hasPassword(const Tp::AccountPtr &account);

    /** Returns the stored password*/
    QString password(const Tp::AccountPtr &account);

    /** Set the password entry for the given account to a new password*/
    void setPassword(const Tp::AccountPtr &account, const QString &password);
private:
    static const QLatin1String s_folderName;

    QScopedPointer<KWallet::Wallet> m_wallet;

};

#endif // WALLETINTERFACE_H
