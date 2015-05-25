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

#include "kaccounts-ktp-plugin.h"

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Profile>
#include <TelepathyQt/ConnectionManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/AccountSet>

#include <KSharedConfig>
#include <KConfigGroup>

#include <QTimer>

#include <KTp/Logger/log-manager.h>

#include <Accounts/Service>
#include <Accounts/Manager>
#include <Accounts/Account>

#include <KAccounts/getcredentialsjob.h>
#include <KAccounts/core.h>

static QStringList s_knownProviders{QStringLiteral("haze-icq"),
                                    QStringLiteral("jabber"),
                                    QStringLiteral("kde-talk"),
                                    QStringLiteral("haze-sametime"),
                                    QStringLiteral("haze-yahoo"),
                                    QStringLiteral("haze-gadugadu")};

class KAccountsKTpPlugin::Private {
public:
    Private(KAccountsKTpPlugin *qq) { q = qq; };
    Tp::AccountPtr tpAccountForAccountId(const Accounts::AccountId accountId);
    void migrateTelepathyAccounts();

    Tp::AccountManagerPtr accountManager;
    Tp::ConnectionManagerPtr connectionManager;
    Tp::ProfilePtr profile;
    KSharedConfigPtr kaccountsConfig;
    KAccountsKTpPlugin *q;
};

Tp::AccountPtr KAccountsKTpPlugin::Private::tpAccountForAccountId(const Accounts::AccountId accountId)
{
    kaccountsConfig->reparseConfiguration();
    KConfigGroup ktpKaccountsGroup = kaccountsConfig->group(QStringLiteral("kaccounts-ktp"));
    QString accountUid = ktpKaccountsGroup.readEntry(QString::number(accountId));

    return accountManager->accountForObjectPath(accountUid);
}

void KAccountsKTpPlugin::Private::migrateTelepathyAccounts()
{
    // some new migration will be needed
}

//---------------------------------------------------------------------------------------

KAccountsKTpPlugin::KAccountsKTpPlugin(QObject *parent)
    : KAccountsDPlugin(parent),
      d(new Private(this))
{
    d->kaccountsConfig = KSharedConfig::openConfig(QStringLiteral("kaccounts-ktprc"));

    Tp::registerTypes();

    // Start setting up the Telepathy AccountManager.
    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                       Tp::Features() << Tp::Account::FeatureCore);

    d->accountManager = Tp::AccountManager::create(accountFactory);
    // There should be well enough time between AM finishes getting ready and before it's needed,
    // so there's no slot watching "finished"
    connect(d->accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

KAccountsKTpPlugin::~KAccountsKTpPlugin()
{
}

void KAccountsKTpPlugin::onAccountManagerReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qWarning() << "Failed initializing AccountManager";
        return;
    }

    d->migrateTelepathyAccounts();
}

void KAccountsKTpPlugin::onAccountCreated(const Accounts::AccountId accountId, const Accounts::ServiceList &serviceList)
{
    //TODO: should we connect the new account here?
}

void KAccountsKTpPlugin::onAccountRemoved(const Accounts::AccountId accountId)
{

}

void KAccountsKTpPlugin::onServiceEnabled(const Accounts::AccountId accountId, const Accounts::Service &service)
{

}

void KAccountsKTpPlugin::onServiceDisabled(const Accounts::AccountId accountId, const Accounts::Service &service)
{

}
