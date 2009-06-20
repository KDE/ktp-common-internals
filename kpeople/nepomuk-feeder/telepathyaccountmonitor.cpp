/*
 * This file is part of telepathy-integration-daemon
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
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

#include <kdebug.h>

#include <TelepathyQt4/PendingReady>

TelepathyAccountMonitor::TelepathyAccountMonitor(QObject *parent)
 : QObject(parent)
{
    // Create an instance of the AccountManager and start to get it ready.
    m_accountManager = Tp::AccountManager::create();

    connect(m_accountManager->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

TelepathyAccountMonitor::~TelepathyAccountMonitor()
{
}

void TelepathyAccountMonitor::onAccountManagerReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Account manager cannot become ready:"
                   << op->errorName() << "-" << op->errorMessage();
        return;
    }

     // Account Manager is now ready. We should watch for any changes in the Accounts List.
    connect(m_accountManager.data(),
            SIGNAL(accountCreated(const QString&)),
            SLOT(onAccountCreated(const QString&)));
    connect(m_accountManager.data(),
            SIGNAL(accountRemoved(const QString&)),
            SLOT(onAccountRemoved(const QString&)));

    foreach (const QString &path, m_accountManager->validAccountPaths()) {
         onAccountCreated(path);
     }
}

void TelepathyAccountMonitor::onAccountCreated(const QString &path)
{
    m_accounts.insert(path, new TelepathyAccount(path, this));
}

void TelepathyAccountMonitor::onAccountRemoved(const QString &path)
{
    Q_UNUSED(path);
    // TODO: Implement me!
}

