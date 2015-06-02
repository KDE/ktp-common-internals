/*
    Copyright (C) 2014  Martin Klapetek <mklapetek@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KACCOUNTSKTPPLUGIN_H
#define KACCOUNTSKTPPLUGIN_H

#include <KAccounts/kaccountsdplugin.h>

namespace Tp {
    class PendingOperation;
}

namespace Accounts {
    class Manager;
}

class KAccountsKTpPlugin : public KAccountsDPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kaccounts.DPlugin")
    Q_INTERFACES(KAccountsDPlugin)

public:
    KAccountsKTpPlugin(QObject *parent = 0);
    ~KAccountsKTpPlugin();

public Q_SLOTS:
    void onAccountCreated(const Accounts::AccountId accountId, const Accounts::ServiceList &serviceList);
    void onAccountRemoved(const Accounts::AccountId accountId);
    void onServiceEnabled(const Accounts::AccountId accountId, const Accounts::Service &service);
    void onServiceDisabled(const Accounts::AccountId accountId, const Accounts::Service &service);

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onAccountSynced();
    void onStorageProviderRetrieved(Tp::PendingOperation *op);

private:
    class Private;
    Private * const d;
};

#endif // KACCOUNTSKTPPLUGIN_H
