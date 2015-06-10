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
#include <TelepathyQt/PendingVariant>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/AccountInterfaceStorageInterface>
#include <TelepathyQt/Utils>
#include <TelepathyQt/PendingVariantMap>

#include <KSharedConfig>
#include <KConfigGroup>

#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QDBusConnection>

#include <KTp/Logger/log-manager.h>

#include <Accounts/Service>
#include <Accounts/Manager>
#include <Accounts/Account>
#include <Accounts/AccountService>
#include <SignOn/IdentityInfo>
#include <SignOn/Identity>

#include <KAccounts/getcredentialsjob.h>
#include <KAccounts/core.h>

// for ::kill
#include <signal.h>

static QStringList s_knownProviders{QStringLiteral("haze-icq"),
                                    QStringLiteral("jabber"),
                                    QStringLiteral("kde-talk"),
                                    QStringLiteral("haze-sametime"),
                                    QStringLiteral("haze-yahoo"),
                                    QStringLiteral("haze-gadugadu")};

class KAccountsKTpPlugin::Private {
public:
    Private(KAccountsKTpPlugin *qq) { q = qq; migrationRef = 0; };
    Tp::AccountPtr tpAccountForAccountId(const Accounts::AccountId accountId);
    void migrateTelepathyAccounts();
    void migrateLogs(const QString &tpAccountId, const Accounts::AccountId accountId);
    void derefMigrationCount();

    Tp::AccountManagerPtr accountManager;
    Tp::ConnectionManagerPtr connectionManager;
    Tp::ProfilePtr profile;
    KSharedConfigPtr kaccountsConfig;
    QString logsBasePath;
    uint migrationRef;
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
    KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral("kaccountsrc"));
    KConfigGroup generalGroup = config->group(QStringLiteral("General"));
    bool migrationDone = generalGroup.readEntry(QStringLiteral("migration2Done"), false);

    if (migrationDone) {
        return;
    }

    auto tpAccountsList = accountManager->validAccounts()->accounts();
    migrationRef = tpAccountsList.size();

    qDebug() << "Starting accounts migration...";

    Q_FOREACH (const Tp::AccountPtr &account, tpAccountsList) {
        KConfigGroup kaccountsKtpGroup = kaccountsConfig->group(QStringLiteral("ktp-kaccounts"));
        const Accounts::AccountId kaccountsId = kaccountsKtpGroup.readEntry(account->objectPath(), 0);

        qDebug() << "Looking at" << account->objectPath();
        qDebug() << " KAccounts id" << kaccountsId;

        if (kaccountsId != 0) {
            migrateLogs(account->objectPath(), kaccountsId);

            Accounts::Account *kaccount = KAccounts::accountsManager()->account(kaccountsId);
            if (!kaccount) {
                qWarning() << "KAccount for" << kaccountsId << "does not exist, removing it from config";
                kaccountsKtpGroup.deleteEntry(account->objectPath());
                KConfigGroup ktpKaccountsGroup = kaccountsConfig->group(QStringLiteral("kaccounts-ktp"));
                ktpKaccountsGroup.deleteEntry(QString::number(kaccountsId));
                derefMigrationCount();
                continue;
            }

            auto services = kaccount->services(QStringLiteral("IM"));

            if (services.size() > 0) {
                qDebug() << "Writing service data:" << account->cmName() << account->protocolName() << account->serviceName();
                Accounts::Service imService = services.at(0);
                Accounts::AccountService accountService(kaccount, imService);
                accountService.setValue("telepathy/manager", account->cmName());
                accountService.setValue("telepathy/protocol", account->protocolName());
                kaccount->sync();
            }

            // Remove the mapping from the config file
            kaccountsKtpGroup.deleteEntry(account->objectPath());
            KConfigGroup ktpKaccountsGroup = kaccountsConfig->group(QStringLiteral("kaccounts-ktp"));
            ktpKaccountsGroup.deleteEntry(QString::number(kaccountsId));

            kaccount->deleteLater();
            // Remove the old Tp Account; the new one will be served by the MC plugin directly
            account->remove();
            derefMigrationCount();
        } else {
            // Get account storage interface for checking if this account is not already handled
            // by Accounts SSO MC plugin
            Tp::Client::AccountInterfaceStorageInterface storageInterface(account.data());
            Tp::PendingVariant *data = storageInterface.requestPropertyStorageProvider();
            data->setProperty("accountObjectPath", account->objectPath());
            QObject::connect(data, &Tp::PendingOperation::finished, q, &KAccountsKTpPlugin::onStorageProviderRetrieved);
        }
    }
}

void KAccountsKTpPlugin::onStorageProviderRetrieved(Tp::PendingOperation *op)
{
    const QString storageProvider = qobject_cast<Tp::PendingVariant*>(op)->result().toString();
    const QString accountObjectPath = op->property("accountObjectPath").toString();
    if (storageProvider == QLatin1String("im.telepathy.Account.Storage.AccountsSSO")) {
        qDebug() << "Found Tp Account" << accountObjectPath << "with AccountsSSO provider, skipping...";
        d->derefMigrationCount();
        return;
    }

    qDebug() << "Creating new KAccounts account for" << accountObjectPath;
    Accounts::Account *kaccount;

    Tp::AccountPtr account = d->accountManager->accountForObjectPath(accountObjectPath);

    if (account.isNull() || !account->isValid()) {
        qDebug() << "An invalid Tp Account retrieved, aborting...";
        d->derefMigrationCount();
        return;
    }

    QString providerName = QStringLiteral("ktp-");

    if (s_knownProviders.contains(account->serviceName())) {
        providerName.append(account->serviceName());
    } else {
        providerName.append(QStringLiteral("generic"));
    }

    qDebug() << "Creating account with providerName" << providerName;

    kaccount = KAccounts::accountsManager()->createAccount(providerName);
    kaccount->setDisplayName(account->displayName());
    kaccount->setValue(QStringLiteral("uid"), account->objectPath());
    kaccount->setValue(QStringLiteral("username"), account->nickname());
    kaccount->setValue(QStringLiteral("auth/mechanism"), QStringLiteral("password"));
    kaccount->setValue(QStringLiteral("auth/method"), QStringLiteral("password"));

    kaccount->setEnabled(true);

    Accounts::ServiceList services = kaccount->services();
    Q_FOREACH(const Accounts::Service &service, services) {
        kaccount->selectService(service);
        kaccount->setEnabled(account->isEnabled());

        if (service.serviceType() == QLatin1String("IM")) {
            // Set the telepathy/ settings on the service so that
            // the MC plugin can use this service
            Accounts::AccountService accountService(kaccount, service);
            accountService.setValue("telepathy/manager", account->cmName());
            accountService.setValue("telepathy/protocol", account->protocolName());
        }
    }

    SignOn::IdentityInfo info;
    info.setUserName(account->nickname());
    // TODO? Query for the current password and store directly?
    // info.setSecret(secret);
    info.setCaption(account->nickname());
    info.setAccessControlList(QStringList(QLatin1String("*")));
    info.setType(SignOn::IdentityInfo::Application);

    SignOn::Identity *identity = SignOn::Identity::newIdentity(info, this);

    if (!identity) {
        qWarning() << "Unable to create new SignOn::Identity, aborting...";
        d->derefMigrationCount();
        return;
    }

    identity->storeCredentials(info);

    kaccount->setCredentialsId(identity->id());
    kaccount->sync();
    QObject::connect(kaccount, &Accounts::Account::synced, this, &KAccountsKTpPlugin::onAccountSynced);
}

void KAccountsKTpPlugin::onAccountSynced()
{
    Accounts::Account *account = qobject_cast<Accounts::Account*>(sender());
    if (!account) {
        d->derefMigrationCount();
        return;
    }

    const QString tpAccountId = account->value(QStringLiteral("uid")).toString();
    d->migrateLogs(tpAccountId, account->id());
    Tp::AccountPtr tpAccount = d->accountManager->accountForObjectPath(tpAccountId);
    // Remove the old Tp Account; the new one will be served by the MC plugin directly
    tpAccount->remove();
    d->derefMigrationCount();
}

void KAccountsKTpPlugin::Private::migrateLogs(const QString &tpAccountId, const Accounts::AccountId accountId)
{
    if (tpAccountId.isEmpty() || accountId == 0) {
        qWarning() << "Cannot finish migration because of empty data received: TP account id:" << tpAccountId << "KAccounts ID:" << accountId;
        return;
    }

    if (logsBasePath.isEmpty()) {
        logsBasePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/TpLogger/logs");
    }

    Tp::AccountPtr tpAccount = accountManager->accountForObjectPath(tpAccountId);

    if (tpAccount.isNull() || !tpAccount->isValid()) {
        qDebug() << "Invalid account for" << tpAccountId << "aborting...";
        return;
    }

    // Construct the new dir which is in form "$cmName_$protocol_ktp_2d$service_name_$KAccountsID"
    // eg. haze_icq_ktp_2d_haze_2dicq_2dim_24
    QString newLogsDir = tpAccount->cmName() + QLatin1Char('_') + tpAccount->protocolName() + QLatin1Char('_');

    if (tpAccount->serviceName() == QLatin1String("google-talk")) {
        newLogsDir += QStringLiteral("google_2dim");
    } else {
        newLogsDir += Tp::escapeAsIdentifier(QStringLiteral("ktp-") + tpAccount->serviceName());
    }

    newLogsDir += QLatin1Char('_') + QString::number(accountId);

    QString accountLogsDir = tpAccount->uniqueIdentifier();

    /* Escape '/' in tpAccountId as '_' */
    if (accountLogsDir.contains(QLatin1Char('/'))) {
        accountLogsDir.replace(QLatin1Char('/'), QLatin1String("_"));
    }

    QDir logsDir(logsBasePath);

    qDebug() << "Migrating logs for" << accountLogsDir << "into" << newLogsDir;

    bool renamed = false;

    if (logsDir.exists()) {
        renamed = logsDir.rename(accountLogsDir, newLogsDir);
    }

    if (!renamed) {
        qWarning() << "Could not rename the directory!";
    }
}

void KAccountsKTpPlugin::Private::derefMigrationCount()
{
    migrationRef--;
    if (migrationRef == 0) {
        qDebug() << "Restarting MC";

        // Get the PID of mission control and then SIGTERM it
        QProcess pidOf;
        pidOf.start(QStringLiteral("pidof"), QStringList() << QStringLiteral("mission-control-5"));
        pidOf.waitForFinished();
        QByteArray pidOfOutput = pidOf.readAllStandardOutput();
        int mcPid = pidOfOutput.trimmed().toInt();
        ::kill(mcPid, SIGTERM);

        QDBusConnection::sessionBus().interface()->startService(QStringLiteral("org.freedesktop.Telepathy.MissionControl5"));

        KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral("kaccountsrc"));
        KConfigGroup generalGroup = config->group(QStringLiteral("General"));
        generalGroup.writeEntry(QStringLiteral("migration2Done"), true);
        generalGroup.sync();

        qDebug() << "Migration done";
    }
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
    Q_UNUSED(accountId);
    Q_UNUSED(serviceList);
    //TODO: should we connect the new account here?
}

void KAccountsKTpPlugin::onAccountRemoved(const Accounts::AccountId accountId)
{
    Q_UNUSED(accountId);
}

void KAccountsKTpPlugin::onServiceEnabled(const Accounts::AccountId accountId, const Accounts::Service &service)
{
    Q_UNUSED(accountId);
    Q_UNUSED(service);
}

void KAccountsKTpPlugin::onServiceDisabled(const Accounts::AccountId accountId, const Accounts::Service &service)
{
    Q_UNUSED(accountId);
    Q_UNUSED(service);
}
