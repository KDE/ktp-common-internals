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

#include "wallet-interface.h"
#include "pending-wallet.h"

#include <KDebug>
#include <KGlobal>


class KTp::WalletInterfacePrivate
{
public:
    WalletInterfacePrivate();
    QScopedPointer<KWallet::Wallet> wallet;
    static const QLatin1String folderName;
    static const QLatin1String mapsPrefix;
};

using KTp::WalletInterface;
using KTp::WalletInterfacePrivate;

const QLatin1String WalletInterfacePrivate::folderName = QLatin1String("telepathy-kde");
const QLatin1String WalletInterfacePrivate::mapsPrefix = QLatin1String("maps/");


WalletInterfacePrivate::WalletInterfacePrivate() :
    wallet(KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), 0, KWallet::Wallet::Asynchronous))
{
}

KTp::PendingWallet* WalletInterface::openWallet()
{
    K_GLOBAL_STATIC(KTp::WalletInterface, s_instance);
    return new PendingWallet(s_instance);
}


WalletInterface::WalletInterface():
    d (new WalletInterfacePrivate)
{
}

WalletInterface::~WalletInterface()
{
    delete d;
}

bool WalletInterface::hasPassword(const Tp::AccountPtr &account)
{
    if (d->wallet.isNull() || !d->wallet->hasFolder(d->folderName)) {
        return false;
    }

    d->wallet->setFolder(d->folderName);
    return d->wallet->hasEntry(account->uniqueIdentifier());
}

QString WalletInterface::password(const Tp::AccountPtr &account)
{
    if (d->wallet.isNull() || !d->wallet->hasFolder(d->folderName)) {
        return QString();
    }

    d->wallet->setFolder(d->folderName);
    QString password;
    if (d->wallet->hasEntry(account->uniqueIdentifier())) {
        int rc = d->wallet->readPassword(account->uniqueIdentifier(), password);
        if (rc != 0) {
            password.clear();
            kWarning() << "failed to read password from KWallet";
        }
    }
    return password;
}

void WalletInterface::setPassword(const Tp::AccountPtr &account, const QString &password)
{
    if (d->wallet.isNull()) {
        return;
    }

    if (!d->wallet->hasFolder(d->folderName)) {
        d->wallet->createFolder(d->folderName);
    }

    d->wallet->setFolder(d->folderName);
    d->wallet->writePassword(account->uniqueIdentifier(), password);

    setLastLoginFailed(account, false);

    //sync normally happens on close, but in this case we need it to happen /now/ as it needs to be synced before the auth-client starts
    d->wallet->sync();
}

void WalletInterface::setLastLoginFailed(const Tp::AccountPtr &account, bool failed)
{
    if (failed) {
        setEntry(account, QLatin1String("lastLoginFailed"), QLatin1String("true"));
    } else {
        if (hasEntry(account, QLatin1String("lastLoginFailed"))) {
            removeEntry(account, QLatin1String("lastLoginFailed"));
        }
    }
}

bool WalletInterface::lastLoginFailed(const Tp::AccountPtr &account)
{
    if (d->wallet.isNull()) {
        return false;
    }
    return hasEntry(account, QLatin1String("lastLoginFailed"));
}

void WalletInterface::removePassword(const Tp::AccountPtr &account)
{
    if (d->wallet.isNull() || !d->wallet->hasFolder(d->folderName)) {
        return;
    }

    d->wallet->setFolder(d->folderName);
    d->wallet->removeEntry(account->uniqueIdentifier());
    d->wallet->sync();
}

bool WalletInterface::hasEntry(const Tp::AccountPtr &account, const QString &key)
{
    if (d->wallet.isNull() || !d->wallet->hasFolder(d->folderName)) {
        return false;
    }

    d->wallet->setFolder(d->folderName);
    QMap< QString, QString > map;
    if (d->wallet->hasEntry(d->mapsPrefix + account->uniqueIdentifier())) {
        int rc = d->wallet->readMap(d->mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return false;
        }
    }
    return map.contains(key);
}

QString WalletInterface::entry(const Tp::AccountPtr &account, const QString &key)
{
    if (d->wallet.isNull() || !d->wallet->hasFolder(d->folderName)) {
        return QString();
    }

    d->wallet->setFolder(d->folderName);
    QString value;
    QMap< QString, QString > map;
    if (d->wallet->hasEntry(d->mapsPrefix + account->uniqueIdentifier())) {
        int rc = d->wallet->readMap(d->mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return QString();
        }
    }
    return map.value(key);
}

void WalletInterface::setEntry(const Tp::AccountPtr &account, const QString &key, const QString &value)
{
    if (d->wallet.isNull()) {
        return;
    }

    if (! d->wallet->hasFolder(d->folderName)) {
        d->wallet->createFolder(d->folderName);
    }

    d->wallet->setFolder(d->folderName);
    QMap< QString, QString > map;
    if (d->wallet->hasEntry(d->mapsPrefix + account->uniqueIdentifier())) {
        int rc = d->wallet->readMap(d->mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return;
        }
    }
    map[key] = value;

    d->wallet->writeMap(d->mapsPrefix + account->uniqueIdentifier(), map);
    //sync normally happens on close, but in this case we need it to happen /now/ as it needs to be synced before the auth-client starts
    d->wallet->sync();
}

void WalletInterface::removeEntry(const Tp::AccountPtr &account, const QString &key)
{
    if (d->wallet.isNull() || !d->wallet->hasFolder(d->folderName)) {
        return;
    }

    d->wallet->setFolder(d->folderName);
    QMap< QString, QString > map;
    if (d->wallet->hasEntry(d->mapsPrefix + account->uniqueIdentifier())) {
        int rc = d->wallet->readMap(d->mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return;
        }
    }
    map.remove(key);

    if (!map.empty()) {
        d->wallet->writeMap(d->mapsPrefix + account->uniqueIdentifier(), map);
    } else {
        d->wallet->removeEntry(d->mapsPrefix + account->uniqueIdentifier());
    }
    //sync normally happens on close, but in this case we need it to happen /now/ as it needs to be synced before the auth-client starts
    d->wallet->sync();
}

void WalletInterface::removeAllEntries(const Tp::AccountPtr& account)
{
    if (d->wallet.isNull() || !d->wallet->hasFolder(d->folderName)) {
        return;
    }

    d->wallet->setFolder(d->folderName);
    d->wallet->removeEntry(d->mapsPrefix + account->uniqueIdentifier());
}

void WalletInterface::removeAccount(const Tp::AccountPtr& account)
{
    removePassword(account);
    removeAllEntries(account);
}

bool WalletInterface::isOpen()
{
    return (!d->wallet.isNull() && d->wallet->isOpen());
}

KWallet::Wallet *WalletInterface::wallet() const
{
    return d->wallet.data();
}


