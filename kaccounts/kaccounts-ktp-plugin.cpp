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
    Accounts::Manager *manager = KAccounts::accountsManager();
    Accounts::Account *account;

    kaccountsConfig->reparseConfiguration();
    KConfigGroup ktpKaccountsGroup = kaccountsConfig->group(QStringLiteral("ktp-kaccounts"));

    qDebug() << "Going to migrate Tp accounts";

    Q_FOREACH (const Tp::AccountPtr &tpAccount, accountManager->validAccounts()->accounts()) {
        if (ktpKaccountsGroup.hasKey(tpAccount->objectPath())) {
            // we already have this account
            continue;
        }

        QString providerName = QStringLiteral("ktp-");

        if (s_knownProviders.contains(tpAccount->serviceName())) {
            providerName.append(tpAccount->serviceName());
        } else {
            providerName.append(QStringLiteral("generic"));
        }

        qDebug() << "Creating account with providerName" << providerName;

        account = manager->createAccount(providerName);
        account->setDisplayName(tpAccount->displayName());
        account->setValue(QStringLiteral("uid"), tpAccount->objectPath());
        account->setValue(QStringLiteral("username"), tpAccount->nickname());
        account->setValue(QStringLiteral("auth/mechanism"), QStringLiteral("password"));
        account->setValue(QStringLiteral("auth/method"), QStringLiteral("password"));

        account->setEnabled(true);

        Accounts::ServiceList services = account->services();
        Q_FOREACH(const Accounts::Service &service, services) {
            account->selectService(service);
            account->setEnabled(tpAccount->isEnabled());
        }

        qDebug() << tpAccount->nickname() << account->id();

        account->sync();
        QObject::connect(account, &Accounts::Account::synced, q, &KAccountsKTpPlugin::onAccountSynced);
    }
}

void KAccountsKTpPlugin::onAccountSynced()
{
    Accounts::Account *account = qobject_cast<Accounts::Account*>(sender());
    if (!account) {
        return;
    }
    KConfigGroup ktpKaccountsGroup = d->kaccountsConfig->group(QStringLiteral("kaccounts-ktp"));
    ktpKaccountsGroup.writeEntry(QString::number(account->id()), account->value(QStringLiteral("uid")).toString());

    KConfigGroup kaccountsKtpGroup = d->kaccountsConfig->group(QStringLiteral("ktp-kaccounts"));
    kaccountsKtpGroup.writeEntry(account->value(QStringLiteral("uid")).toString(), QString::number(account->id()));

    d->kaccountsConfig->sync();
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

    // Do a cleanup
    KConfigGroup ktpKaccountsGroup = d->kaccountsConfig->group(QStringLiteral("kaccounts-ktp"));

    auto kaccountsList = KAccounts::accountsManager()->accountList();

    Q_FOREACH (const QString &kaccountId, ktpKaccountsGroup.keyList()) {
        if (!kaccountsList.contains(kaccountId.toUInt())) {
            onAccountRemoved(kaccountId.toUInt());
        }
    }

    d->migrateTelepathyAccounts();
}

void KAccountsKTpPlugin::onAccountCreated(const Accounts::AccountId accountId, const Accounts::ServiceList &serviceList)
{
    bool containsImService = false;
    QString providerName;

    Q_FOREACH (const Accounts::Service &s, serviceList) {
        if (s.serviceType() == QLatin1String("IM")) {
            containsImService = true;
            providerName = s.provider();
            break;
        }
    }

    if (!containsImService) {
        qDebug() << "No IM service found, ignoring...";
        return;
    }

    // if the provider name starts with "ktp-", it means the ktp-accounts UI was used for creating it
    // and it also means it was already created, do nothing.
    if (providerName.startsWith(QLatin1String("ktp-"))) {
        return;
    }

    if (providerName.contains(QLatin1String("google"))) {
        providerName = QStringLiteral("google-talk");
    }

    qDebug() << "Creating new Tp account for AccountId" << accountId;

    // Sometimes it can happen that the database is not yet synced with signond
    // and requesting data from it (GetCredentialsJob in onConnectionManagerReady)
    // may result in an error, so let's give it some grace time and only
    // then go ahead and try getting the info.
    QTimer *delayTimer = new QTimer(this);
    delayTimer->setSingleShot(true);

    connect(delayTimer, &QTimer::timeout, [=]() {
        d->profile = Tp::Profile::createForServiceName(providerName);

        d->connectionManager = Tp::ConnectionManager::create(d->profile->cmName());
        Tp::PendingReady *op = d->connectionManager->becomeReady();
        op->setProperty("accountId", accountId);
        connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                this, SLOT(onConnectionManagerReady(Tp::PendingOperation*)));

        delayTimer->deleteLater();
    });

    delayTimer->start(1500);
}

void KAccountsKTpPlugin::onConnectionManagerReady(Tp::PendingOperation *op)
{
    quint32 accountId = op->property("accountId").toUInt();
    GetCredentialsJob *credentialsJob = new GetCredentialsJob(accountId, QStringLiteral("password"), QStringLiteral("password"), this);
    connect(credentialsJob, &GetCredentialsJob::finished, [this, accountId](KJob *job) {

        if (job->error()) {
            qWarning() << "Failed at receiving credentials, aborting creating new Telepathy account";
            return;
        }

        Tp::ProtocolInfo protocolInfo = d->connectionManager->protocol(d->profile->protocolName());
        Tp::ProtocolParameterList parameters = protocolInfo.parameters();
        Tp::Profile::ParameterList profileParameters = d->profile->parameters();

        QVariantMap credentials = qobject_cast<GetCredentialsJob*>(job)->credentialsData();
        QVariantMap values;

        Q_FOREACH (const Tp::ProtocolParameter &parameter, parameters) {
            //try and find the correct profile parameter, if it can't be found leave it as empty.
            Q_FOREACH (const Tp::Profile::Parameter &profileParameter, profileParameters) {
                if (profileParameter.name() == parameter.name()) {
                    values.insert(parameter.name(), profileParameter.value());
                    break;
                }
            }
        }

        values.insert(QStringLiteral("account"), credentials.value(QStringLiteral("UserName")));

        // FIXME: In some next version of tp-qt4 there should be a convenience class for this
        // https://bugs.freedesktop.org/show_bug.cgi?id=33153
        QVariantMap properties;

        if (d->accountManager->supportedAccountProperties().contains(QLatin1String("org.freedesktop.Telepathy.Account.Service"))) {
            properties.insert(QLatin1String("org.freedesktop.Telepathy.Account.Service"), d->profile->serviceName());
        }
        if (d->accountManager->supportedAccountProperties().contains(QLatin1String("org.freedesktop.Telepathy.Account.Enabled"))) {
            properties.insert(QLatin1String("org.freedesktop.Telepathy.Account.Enabled"), true);
        }

        qDebug() << "Sending account manager request to create new account";

        Tp::PendingAccount *pa = d->accountManager->createAccount(d->profile->cmName(),
                                                                  d->profile->protocolName(),
                                                                  credentials.value(QStringLiteral("UserName")).toString(),
                                                                  values,
                                                                  properties);

        connect(pa,
                &Tp::PendingAccount::finished, [this, accountId](Tp::PendingOperation *op) {
                    if (op->isError()) {
                        qWarning() << "Failed to create KDE Telepathy account -" << op->errorName() << op->errorMessage();
                    } else {
                        Tp::PendingAccount *pendingAccount = qobject_cast<Tp::PendingAccount*>(op);
                        if (!pendingAccount) {
                            qWarning() << "Cannot cast operation to PendingAccount!";
                            return;
                        }
                        KConfigGroup ktpKaccountsGroup = d->kaccountsConfig->group(QStringLiteral("kaccounts-ktp"));
                        ktpKaccountsGroup.writeEntry(QString::number(accountId), pendingAccount->account()->objectPath());

                        KConfigGroup kaccountsKtpGroup = d->kaccountsConfig->group(QStringLiteral("ktp-kaccounts"));
                        kaccountsKtpGroup.writeEntry(pendingAccount->account()->objectPath(), accountId);

                        d->kaccountsConfig->sync();
                    }
                });

    });

    credentialsJob->start();
}

void KAccountsKTpPlugin::onAccountRemoved(const Accounts::AccountId accountId)
{
    d->kaccountsConfig->group(QStringLiteral("kaccounts-ktp")).keyList();
    // Lookup the config file and then proceed to remove the Tp account
    // that corresponds with the account id
    Tp::AccountPtr account = d->tpAccountForAccountId(accountId);

    // Delete the entry from config file
    KConfigGroup ktpKaccountsGroup = d->kaccountsConfig->group(QStringLiteral("kaccounts-ktp"));
    // Read it first so we can then remove the reversed entry
    QString accountUid = ktpKaccountsGroup.readEntry(QString::number(accountId));
    ktpKaccountsGroup.deleteEntry(QString::number(accountId));

    // As the config file contains mapping both ways (ktp id -> accounts id; accounts id -> ktp id)
    // we also need to remove the other entry
    KConfigGroup kaccountsKtpGroup = d->kaccountsConfig->group(QStringLiteral("ktp-kaccounts"));
    kaccountsKtpGroup.deleteEntry(accountUid);

    d->kaccountsConfig->sync();

    if (account.isNull()) {
        qWarning() << "Account manager returned null account, aborting";
        return;
    }

    // FIXME keep this non-optional? The problem is that we can't show the "are you sure"
    //       dialog here as that's too late at this point
    KTp::LogManager *logManager = KTp::LogManager::instance();
    logManager->clearAccountLogs(account);

    account->remove();
}

void KAccountsKTpPlugin::onServiceEnabled(const Accounts::AccountId accountId, const Accounts::Service &service)
{
    if (service.serviceType() != QLatin1String("IM")) {
        return;
    }

    Tp::AccountPtr account = d->tpAccountForAccountId(accountId);

    if (account.isNull()) {
        qWarning() << "Account manager returned null account, aborting";
        return;
    }

    Tp::PendingOperation *op = account->setEnabled(true);
    connect(op, &Tp::PendingOperation::finished, [](Tp::PendingOperation *op) {
        if (op->isError()) {
            qWarning() << "Unable to enable account -" << op->errorName() << op->errorMessage();
        }
    });
}

void KAccountsKTpPlugin::onServiceDisabled(const Accounts::AccountId accountId, const Accounts::Service &service)
{
    if (service.serviceType() != QLatin1String("IM")) {
        return;
    }

    Tp::AccountPtr account = d->tpAccountForAccountId(accountId);

    if (account.isNull()) {
        qWarning() << "Account manager returned null account, aborting";
        return;
    }

    Tp::PendingOperation *op = account->setEnabled(false);
    connect(op, &Tp::PendingOperation::finished, [](Tp::PendingOperation *op) {
        if (op->isError()) {
            qWarning() << "Unable to disable account -" << op->errorName() << op->errorMessage();
        }
    });
}
