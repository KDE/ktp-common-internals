/*
 * This file is part of telepathy-nepomuk-service
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

#ifndef NEPOMUK_TELEPATHY_SERVICE_CONTROLLER_H
#define NEPOMUK_TELEPATHY_SERVICE_CONTROLLER_H

#include <QtCore/QObject>

#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>

namespace Tp {
    class PendingOperation;
}

class AbstractStorage;

/**
 * Acts as the controller part of a MVC based system (with Storage and the Account/Channel/Contact
 * wrapper classes acting as the rest of the system).
 *
 * This class monitors the Telepathy AccountManager and ensures that Account wrappers are created
 * for every account on it.
 */
class Controller : public QObject
{
    Q_OBJECT

public:
    explicit Controller(AbstractStorage *storage, QObject *parent = 0);
    ~Controller();

    void shutdown();

Q_SIGNALS:
    void storageInitialisationFailed();

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onNewAccount(const Tp::AccountPtr &account);
    void onStorageInitialised(bool success);

private:
    Q_DISABLE_COPY(Controller);

    AbstractStorage *m_storage;

    Tp::AccountManagerPtr m_accountManager;
};


#endif // Header guard

