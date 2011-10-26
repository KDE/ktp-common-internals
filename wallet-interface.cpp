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

using KTelepathy::WalletInterface;

const QLatin1String WalletInterface::s_folderName = QLatin1String("telepathy-kde");
const QLatin1String WalletInterface::s_mapsPrefix = QLatin1String("maps/");


WalletInterface::WalletInterface(WId winId):
    m_wallet(KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), winId))
{
}

WalletInterface::~WalletInterface()
{
}

bool WalletInterface::hasPassword(const Tp::AccountPtr &account) const
{
    if (m_wallet.isNull() || !m_wallet->hasFolder(s_folderName)) {
        return false;
    }

    m_wallet->setFolder(s_folderName);
    return m_wallet->hasEntry(account->uniqueIdentifier());
}

QString WalletInterface::password(const Tp::AccountPtr &account) const
{
    if (m_wallet.isNull() || !m_wallet->hasFolder(s_folderName)) {
        return QString();
    }

    m_wallet->setFolder(s_folderName);
    QString password;
    if (m_wallet->hasEntry(account->uniqueIdentifier())) {
        int rc = m_wallet->readPassword(account->uniqueIdentifier(), password);
        if (rc != 0) {
            password.clear();
            kWarning() << "failed to read password from KWallet";
        }
    }
    return password;
}

void WalletInterface::setPassword(const Tp::AccountPtr &account, const QString &password)
{
    if (m_wallet.isNull()) {
        return;
    }

    if (!m_wallet->hasFolder(s_folderName)) {
        m_wallet->createFolder(s_folderName);
    }

    m_wallet->setFolder(s_folderName);
    m_wallet->writePassword(account->uniqueIdentifier(), password);
    //sync normally happens on close, but in this case we need it to happen /now/ as it needs to be synced before the auth-client starts
    m_wallet->sync();
}

void WalletInterface::removePassword(const Tp::AccountPtr &account)
{
    if (m_wallet.isNull() || !m_wallet->hasFolder(s_folderName)) {
        return;
    }

    m_wallet->setFolder(s_folderName);
    m_wallet->removeEntry(account->uniqueIdentifier());
    m_wallet->sync();
}

bool WalletInterface::hasEntry(const Tp::AccountPtr &account, const QString &key) const
{
    if (m_wallet.isNull() || !m_wallet->hasFolder(s_folderName)) {
        return false;
    }

    m_wallet->setFolder(s_folderName);
    QMap< QString, QString > map;
    if (m_wallet->hasEntry(s_mapsPrefix + account->uniqueIdentifier())) {
        int rc = m_wallet->readMap(s_mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return false;
        }
    }
    return map.contains(key);
}

QString WalletInterface::entry(const Tp::AccountPtr &account, const QString &key) const
{
    if (m_wallet.isNull() || !m_wallet->hasFolder(s_folderName)) {
        return QString();
    }

    m_wallet->setFolder(s_folderName);
    QString value;
    QMap< QString, QString > map;
    if (m_wallet->hasEntry(s_mapsPrefix + account->uniqueIdentifier())) {
        int rc = m_wallet->readMap(s_mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return QString();
        }
    }
    return map.value(key);
}

void WalletInterface::setEntry(const Tp::AccountPtr &account, const QString &key, const QString &value)
{
    if (m_wallet.isNull()) {
        return;
    }

    if (! m_wallet->hasFolder(s_folderName)) {
        m_wallet->createFolder(s_folderName);
    }

    m_wallet->setFolder(s_folderName);
    QMap< QString, QString > map;
    if (m_wallet->hasEntry(s_mapsPrefix + account->uniqueIdentifier())) {
        int rc = m_wallet->readMap(s_mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return;
        }
    }
    map[key] = value;

    m_wallet->writeMap(s_mapsPrefix + account->uniqueIdentifier(), map);
    //sync normally happens on close, but in this case we need it to happen /now/ as it needs to be synced before the auth-client starts
    m_wallet->sync();
}

void WalletInterface::removeEntry(const Tp::AccountPtr &account, const QString &key)
{
    if (m_wallet.isNull() || !m_wallet->hasFolder(s_folderName)) {
        return;
    }

    m_wallet->setFolder(s_folderName);
    QMap< QString, QString > map;
    if (m_wallet->hasEntry(s_mapsPrefix + account->uniqueIdentifier())) {
        int rc = m_wallet->readMap(s_mapsPrefix + account->uniqueIdentifier(), map);
        if (rc != 0) {
            kWarning() << "failed to read map from KWallet (probably it is not a map)";
            return;
        }
    }
    map.remove(key);

    if (!map.empty()) {
        m_wallet->writeMap(s_mapsPrefix + account->uniqueIdentifier(), map);
    } else {
        m_wallet->removeEntry(s_mapsPrefix + account->uniqueIdentifier());
    }
    //sync normally happens on close, but in this case we need it to happen /now/ as it needs to be synced before the auth-client starts
    m_wallet->sync();
}

void WalletInterface::removeAllEntries(const Tp::AccountPtr& account)
{
    if (m_wallet.isNull() || !m_wallet->hasFolder(s_folderName)) {
        return;
    }

    m_wallet->setFolder(s_folderName);
    m_wallet->removeEntry(s_mapsPrefix + account->uniqueIdentifier());
}
