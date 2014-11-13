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

#include <TelepathyQt/Account>
#include <TelepathyQt/PendingOperation>

#include <KWallet/KWallet>

#include <QScopedPointer>

#include <KTp/ktpcommoninternals_export.h>

namespace KTp
{

class WalletInterfacePrivate;
class PendingWallet;


/** Class wraps interface around KWallet. A singleton is used to make sure that the wallet is only even opened once if multiple instances of
    this class are used throughout the application*/

class KTPCOMMONINTERNALS_EXPORT WalletInterface
{
    friend class WalletInterfacePrivate;
public:

    /** Tries to open the wallet. Op contains a pointer to the wallet operation interface*/
    static KTp::PendingWallet *openWallet();

    /** Returns true if a password is stored for the given account */
    bool hasPassword(const Tp::AccountPtr &account);

    /** Returns the stored password for the given account */
    QString password(const Tp::AccountPtr &account);

    /** Set the password for the given account to a new password */
    void setPassword(const Tp::AccountPtr &account, const QString &password);

    /** Sets whether using the password failed */
    void setLastLoginFailed(const Tp::AccountPtr &account, bool failed);

    /** Returns whether using the password failed */
    bool lastLoginFailed(const Tp::AccountPtr &account);

    /** Remove the password for the given account from kwallet */
    void removePassword(const Tp::AccountPtr &account);

    /** Returns true if a given entry is stored for the given account */
    bool hasEntry(const Tp::AccountPtr &account, const QString &key);

    /** Returns the stored entry for the given account */
    QString entry(const Tp::AccountPtr &account, const QString &key);

    /** Set an entry for the given account to a new value */
    void setEntry(const Tp::AccountPtr &account, const QString &key, const QString &value);

    /** Remove the entry for the given account from kwallet */
    void removeEntry(const Tp::AccountPtr &account, const QString &key);

    /** Remove all the entries for the given account from kwallet */
    void removeAllEntries(const Tp::AccountPtr &account);

    /** Remove entries and password for the account from kwallet */
    void removeAccount(const Tp::AccountPtr &account);

    /** Determine if the wallet is open, and is a valid wallet handle */
    bool isOpen();

    /** Raw access to the underlying wallet. Should not be used.*/
    KWallet::Wallet* wallet() const;

private:
    Q_DISABLE_COPY(WalletInterface)
    WalletInterface();
    virtual ~WalletInterface();
    WalletInterfacePrivate * const d;
};

} // namespace KTp

#endif // WALLETINTERFACE_H
