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

#include <KSharedConfig>
#include <KConfigGroup>

#include <KTp/Logger/log-manager.h>

class KAccountsKTpPlugin::Private {
public:
    Tp::AccountManagerPtr accountManager;
};

KAccountsKTpPlugin::KAccountsKTpPlugin(QObject *parent)
    : KAccountsDPlugin(parent),
      d(new Private)
{
    Tp::registerTypes();

    // Start setting up the Telepathy AccountManager.
    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                       Tp::Features() << Tp::Account::FeatureCore);

    d->accountManager = Tp::AccountManager::create(accountFactory);
    // There should be well enough time between AM finishes getting ready and before it's needed,
    // so there's no slot watching "finished"
    d->accountManager->becomeReady();
}

KAccountsKTpPlugin::~KAccountsKTpPlugin()
{

}

void KAccountsKTpPlugin::onAccountCreated(const Accounts::AccountId accountId, const Accounts::ServiceList &serviceList)
{
    // TODO:
    // Here will go account services setting up, eg. if one adds Google account
    // in KAccounts, we need to check if he requested "Chat" service here and
    // set up the account here
}

void KAccountsKTpPlugin::onAccountRemoved(const Accounts::AccountId accountId)
{
    // Lookup the config file and then proceed to remove the Tp account
    // that corresponds with the account id
    KSharedConfigPtr kaccountsConfig = KSharedConfig::openConfig(QStringLiteral("kaccounts-ktprc"));
    KConfigGroup ktpKaccountsGroup = kaccountsConfig->group(QStringLiteral("kaccounts-ktp"));
    QString accountUid = ktpKaccountsGroup.readEntry(QString::number(accountId));

    if (accountUid.isEmpty()) {
        qWarning() << "The config file returned emtpy account uid, aborting";
        return;
    }

    Tp::AccountPtr account = d->accountManager->accountForObjectPath(accountUid);
    if (account.isNull()) {
        qWarning() << "Account manager returned invalid account, aborting";
        return;
    }

    // FIXME keep this non-optional? The problem is that we can't show the "are you sure"
    //       dialog here as that's too late at this point
    if (1) {
        KTp::LogManager *logManager = KTp::LogManager::instance();
        logManager->clearAccountLogs(account);
    }

    qDebug() << "Removing Telepathy account with object path" << account->displayName();
    account->remove();

    // Delete the entry from config file
    ktpKaccountsGroup.deleteEntry(QString::number(accountId));

    // As the config file contains mapping both ways (ktp id -> accounts id; accounts id -> ktp id)
    // we also need to remove the other entry
    ktpKaccountsGroup = kaccountsConfig->group(QStringLiteral("ktp-kaccounts"));
    ktpKaccountsGroup.deleteEntry(accountUid);
}

void KAccountsKTpPlugin::onServiceEnabled(const Accounts::AccountId accountId, const Accounts::Service& service)
{

}

void KAccountsKTpPlugin::onServiceDisabled(const Accounts::AccountId accountId, const Accounts::Service& service)
{

}
