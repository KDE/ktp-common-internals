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

K_GLOBAL_STATIC(KTp::WalletInterfacePrivate, instance)

WalletInterfacePrivate::WalletInterfacePrivate() :
    wallet(KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), 0))
{
}

bool WalletInterface::hasPassword(const Tp::AccountPtr &account)
{
    if (instance->wallet.isNull() || !instance->wallet->hasFolder(instance->folderName)) {
        return false;
    }

    instance->wallet->setFolder(instance->folderName);
    return instance->wallet->hasEntry(account->uniqueIdentifier());
}

QString WalletInterface::password(const Tp::AccountPtr &account)
{
    if (instance->wallet.isNull() || !instance->wallet->hasFolder(instance->folderName)) {
        return QString();
    }

    instance->wallet->setFolder(instance->folderName);
    QString password;
    if (instance->wallet->hasEntry(account->uniqueIdentifier())) {
        int rc = instance->wallet->readPassword(account->uniqueIdentifier(), password);
        if (rc != 0) {
            password.clear();
            kWarning() << "failed to read password from KWallet";
        }
    }
    return password;
}

void WalletInterface::setPassword(const Tp::AccountPtr &account, const QString &password)
{
    if (instance->wallet.isNull()) {
        return;
    }

    if (!instance->wallet->hasFolder(instance->folderName)) {
        instance->wallet->createFolder(instance->folderName);
    }

    instance->wallet->setFolder(instance->folderName);
    instance->wallet->writePassword(account->uniqueIdentifier(), password);
    //sync normally happens on close, but in this case we need it to happen /now/ as it needs to be synced before the auth-client starts
    instance->wallet->sync();
}

void WalletInterface::removePassword(const Tp::AccountPtr &account)
{
    if (instance->wallet.isNull() || !instance->wallet->hasFolder(instance->folderName)) {
        return;
    }

    instance->wallet->setFolder(instance->folderName);
    instance->wallet->removeEntry(account->uniqueIdentifier());
    instance->wallet->sync();
}

bool WalletInterface::hasEntry(const Tp::AccountPtr &account, const QString &key)
{
    if (instance->wallet.isNull() || !instance->wallet->hasFolder(instance->folderName)) {
        return false;
    }

    instance->wallet->setFolder(instance->folderName);
    QMap< QString, QString > map;
    if (instance->wallet->hasEntry(instance->mapsPrefix + account->uniqueIdentifier())) {
        int rc = instance->wallet->readMap(instance->mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return false;
        }
    }
    return map.contains(key);
}

QString WalletInterface::entry(const Tp::AccountPtr &account, const QString &key)
{
    if (instance->wallet.isNull() || !instance->wallet->hasFolder(instance->folderName)) {
        return QString();
    }

    instance->wallet->setFolder(instance->folderName);
    QString value;
    QMap< QString, QString > map;
    if (instance->wallet->hasEntry(instance->mapsPrefix + account->uniqueIdentifier())) {
        int rc = instance->wallet->readMap(instance->mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return QString();
        }
    }
    return map.value(key);
}

void WalletInterface::setEntry(const Tp::AccountPtr &account, const QString &key, const QString &value)
{
    if (instance->wallet.isNull()) {
        return;
    }

    if (! instance->wallet->hasFolder(instance->folderName)) {
        instance->wallet->createFolder(instance->folderName);
    }

    instance->wallet->setFolder(instance->folderName);
    QMap< QString, QString > map;
    if (instance->wallet->hasEntry(instance->mapsPrefix + account->uniqueIdentifier())) {
        int rc = instance->wallet->readMap(instance->mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return;
        }
    }
    map[key] = value;

    instance->wallet->writeMap(instance->mapsPrefix + account->uniqueIdentifier(), map);
    //sync normally happens on close, but in this case we need it to happen /now/ as it needs to be synced before the auth-client starts
    instance->wallet->sync();
}

void WalletInterface::removeEntry(const Tp::AccountPtr &account, const QString &key)
{
    if (instance->wallet.isNull() || !instance->wallet->hasFolder(instance->folderName)) {
        return;
    }

    instance->wallet->setFolder(instance->folderName);
    QMap< QString, QString > map;
    if (instance->wallet->hasEntry(instance->mapsPrefix + account->uniqueIdentifier())) {
        int rc = instance->wallet->readMap(instance->mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return;
        }
    }
    map.remove(key);

    if (!map.empty()) {
        instance->wallet->writeMap(instance->mapsPrefix + account->uniqueIdentifier(), map);
    } else {
        instance->wallet->removeEntry(instance->mapsPrefix + account->uniqueIdentifier());
    }
    //sync normally happens on close, but in this case we need it to happen /now/ as it needs to be synced before the auth-client starts
    instance->wallet->sync();
}

void WalletInterface::removeAllEntries(const Tp::AccountPtr& account)
{
    if (instance->wallet.isNull() || !instance->wallet->hasFolder(instance->folderName)) {
        return;
    }

    instance->wallet->setFolder(instance->folderName);
    instance->wallet->removeEntry(instance->mapsPrefix + account->uniqueIdentifier());
}

void WalletInterface::removeAccount(const Tp::AccountPtr& account)
{
    removePassword(account);
    removeAllEntries(account);
}

bool WalletInterface::isOpen()
{
    return (!instance->wallet.isNull() && instance->wallet->isOpen());
}
