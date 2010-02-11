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

#ifndef TELEPATHY_INTEGRATION_DAEMON_TELEPATHYACCOUNTMONITOR_H
#define TELEPATHY_INTEGRATION_DAEMON_TELEPATHYACCOUNTMONITOR_H

#include "telepathyaccount.h"

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>

#include <TelepathyQt4/AccountManager>

namespace Tp {
    class PendingOperation;
}

/**
 * Monitors the Telepathy Account Manager, ensuring that whenever an account is created, a
 * TelepathyAccount object is created to monitor it.
 */
class TelepathyAccountMonitor : public QObject
{
    Q_OBJECT

public:
    explicit TelepathyAccountMonitor(QObject *parent = 0);
    ~TelepathyAccountMonitor();

    Tp::AccountManagerPtr accountManager() const;

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onAccountCreated(const QString &path);
    void onNepomukError(const QString &uri, int errorCode);

private:
    Q_DISABLE_COPY(TelepathyAccountMonitor);

    Tp::AccountManagerPtr m_accountManager;

};


#endif // Header guard

