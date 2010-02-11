/*
 * This file is part of telepathy-integration-daemon
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
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

#include "telepathyaccountmonitor.h"
#include <Nepomuk/ResourceManager>

#include <KDebug>


#include <QtCore/QString>
#include <TelepathyQt4/PendingReady>

TelepathyAccountMonitor::TelepathyAccountMonitor(QObject *parent)
 : QObject(parent)
{
    // Create an instance of the AccountManager and start to get it ready.
    m_accountManager = Tp::AccountManager::create();

    connect(m_accountManager->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    Nepomuk::ResourceManager *nepomukResourceManager = Nepomuk::ResourceManager::instance();

    connect(nepomukResourceManager,
            SIGNAL(error(QString, int)),
            SLOT(onNepomukError(QString, int)));
}

TelepathyAccountMonitor::~TelepathyAccountMonitor()
{
}

void TelepathyAccountMonitor::onAccountManagerReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Account manager cannot become ready:"
                   << op->errorName()
                   << "-"
                   << op->errorMessage();
        return;
    }

     // Account Manager is now ready. We should watch for any new accounts being created.
    connect(m_accountManager.data(),
            SIGNAL(accountCreated(const QString&)),
            SLOT(onAccountCreated(const QString&)));

    // Take into account (ha ha) the accounts that already existed when the AM object became ready.
    foreach (const QString &path, m_accountManager->allAccountPaths()) {
         onAccountCreated(path);
     }
}

void TelepathyAccountMonitor::onAccountCreated(const QString &path)
{
    new TelepathyAccount(path, this);
}

Tp::AccountManagerPtr TelepathyAccountMonitor::accountManager() const
{
    return m_accountManager;
}

void TelepathyAccountMonitor::onNepomukError(const QString &uri, int errorCode)
{
    kDebug() << "A Nepomuk Error occurred:" << uri << errorCode;
}


#include "telepathyaccountmonitor.moc"

