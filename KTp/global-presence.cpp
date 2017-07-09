/*
 * Global Presence - wraps calls to set and get presence for all accounts.
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

#include "global-presence.h"

#include <KTp/presence.h>

#include <QDBusPendingCall>
#include <QVariant>

#include <TelepathyQt/Account>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Types>

#include "types.h"
#include "ktp-debug.h"

namespace KTp
{

GlobalPresence::GlobalPresence(QObject *parent)
    : QObject(parent),
      m_connectionStatus(GlobalPresence::Disconnected),
      m_changingPresence(false),
      m_hasEnabledAccounts(false)
{
    Tp::registerTypes();

    m_statusHandlerInterface = new QDBusInterface(QLatin1String("org.freedesktop.Telepathy.Client.KTp.KdedIntegrationModule"),
								  QLatin1String("/StatusHandler"),
								  QString(),
								  QDBusConnection::sessionBus(), this);

    m_requestedPresence.setStatus(Tp::ConnectionPresenceTypeUnset, QLatin1String("unset"), QString());
    m_currentPresence.setStatus(Tp::ConnectionPresenceTypeUnset, QLatin1String("unset"), QString());
}

void GlobalPresence::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    m_accountManager = accountManager;

    m_enabledAccounts = m_accountManager->enabledAccounts();
    m_onlineAccounts = m_accountManager->onlineAccounts();

    for (const Tp::AccountPtr &account : m_enabledAccounts->accounts()) {
        onAccountEnabledChanged(account);
    }

    connect(m_enabledAccounts.data(), &Tp::AccountSet::accountAdded, this, &GlobalPresence::onAccountEnabledChanged);
    connect(m_enabledAccounts.data(), &Tp::AccountSet::accountRemoved, this, &GlobalPresence::onAccountEnabledChanged);

    if (!m_accountManager->isReady()) {
        qCWarning(KTP_COMMONINTERNALS) << "GlobalPresence used with unready account manager";
    } else {
        Q_EMIT accountManagerReady();
    }
}

void GlobalPresence::addAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    connect(accountManager->becomeReady(), &Tp::PendingReady::finished, [=] (Tp::PendingOperation* op) {
        if (op->isError()) {
            qCDebug(KTP_COMMONINTERNALS) << op->errorName();
            qCDebug(KTP_COMMONINTERNALS) << op->errorMessage();

            qCDebug(KTP_COMMONINTERNALS) << "Something unexpected happened to"
              << "the core part of your Instant Messaging system and it couldn't"
              << "be initialized. Try restarting the client.";

            return;
        }

        setAccountManager(accountManager);
    });
}

Tp::AccountManagerPtr GlobalPresence::accountManager() const
{
    return m_accountManager;
}

GlobalPresence::ConnectionStatus GlobalPresence::connectionStatus() const
{
    return m_connectionStatus;
}

KTp::Presence GlobalPresence::currentPresence() const
{
    return m_currentPresence;
}

QString GlobalPresence::currentPresenceMessage() const
{
    return m_currentPresence.statusMessage();
}

QIcon GlobalPresence::currentPresenceIcon() const
{
    return m_currentPresence.icon();
}

QString GlobalPresence::currentPresenceIconName() const
{
    return m_currentPresence.iconName();
}

QString GlobalPresence::currentPresenceName() const
{
    return m_currentPresence.displayString();
}

GlobalPresence::ConnectionPresenceType GlobalPresence::currentPresenceType() const
{
    switch(m_currentPresence.type()) {
        case Tp::ConnectionPresenceTypeAvailable:
            return GlobalPresence::Available;
        case Tp::ConnectionPresenceTypeBusy:
            return GlobalPresence::Busy;
        case Tp::ConnectionPresenceTypeAway:
            return GlobalPresence::Away;
        case Tp::ConnectionPresenceTypeExtendedAway:
            return GlobalPresence::ExtendedAway;
        case Tp::ConnectionPresenceTypeHidden:
            return GlobalPresence::Hidden;
        case Tp::ConnectionPresenceTypeOffline:
            return GlobalPresence::Offline;
        default:
            return GlobalPresence::Unknown;
    }
}

KTp::Presence GlobalPresence::requestedPresence() const
{
    return m_requestedPresence;
}

QString GlobalPresence::requestedPresenceName() const
{
    return m_requestedPresence.displayString();
}

bool GlobalPresence::isChangingPresence() const
{
    return m_changingPresence;
}

KTp::Presence GlobalPresence::globalPresence() const
{
    KTp::Presence globalPresence;
    globalPresence.setStatus(Tp::ConnectionPresenceTypeUnset, QLatin1String("unset"), QString());

    if (m_statusHandlerInterface->property("requestedGlobalPresence").isValid()) {
        globalPresence = KTp::Presence(qdbus_cast<Tp::SimplePresence>(m_statusHandlerInterface->property("requestedGlobalPresence")));
    }

    return globalPresence;
}

void GlobalPresence::setPresence(const KTp::Presence &presence, PresenceClass presenceClass)
{
    if (m_enabledAccounts.isNull()) {
        qCWarning(KTP_COMMONINTERNALS) << "Requested presence change on empty accounts set";
        return;
    }

    if (!presence.isValid()) {
        qCWarning(KTP_COMMONINTERNALS) << "Invalid requested presence";
        return;
    }

    QDBusPendingCall call = m_statusHandlerInterface->asyncCall(QLatin1String("setRequestedGlobalPresence"), QVariant::fromValue<Tp::SimplePresence>(presence.barePresence()), QVariant::fromValue<uint>(presenceClass));
}

void GlobalPresence::setPresence(GlobalPresence::ConnectionPresenceType type, QString message, PresenceClass presenceClass)
{
    KTp::Presence presence;

    switch (type) {
        case GlobalPresence::Available:
            presence = KTp::Presence::available(message);
            break;
        case GlobalPresence::Busy:
            presence = KTp::Presence::busy(message);
            break;
        case GlobalPresence::Away:
            presence = KTp::Presence::away(message);
            break;
        case GlobalPresence::ExtendedAway:
            presence = KTp::Presence::xa(message);
            break;
        case GlobalPresence::Hidden:
            presence = KTp::Presence::hidden(message);
            break;
        case GlobalPresence::Offline:
            presence = KTp::Presence::offline(message);
            break;
        case GlobalPresence::Unknown:
            presence = KTp::Presence(Tp::Presence(Tp::ConnectionPresenceTypeUnknown, QLatin1String("unknown"), message));
            break;
        case GlobalPresence::Unset:
            presence = KTp::Presence(Tp::Presence(Tp::ConnectionPresenceTypeUnset, QLatin1String("unset"), message));
            break;
        default:
            qCDebug(KTP_COMMONINTERNALS) << "You should not be here!";
    }

    setPresence(presence, presenceClass);
}

void GlobalPresence::onAccountEnabledChanged(const Tp::AccountPtr &account)
{
    if (account->isEnabled()) {
        connect(account.data(), &Tp::Account::connectionStatusChanged, this, &GlobalPresence::onConnectionStatusChanged);
        connect(account.data(), &Tp::Account::changingPresence, this, &GlobalPresence::onChangingPresence);
        connect(account.data(), &Tp::Account::requestedPresenceChanged, this, &GlobalPresence::onRequestedPresenceChanged);
        connect(account.data(), &Tp::Account::currentPresenceChanged, this, &GlobalPresence::onCurrentPresenceChanged);
    } else {
        disconnect(account.data());
    }

    onCurrentPresenceChanged(account->currentPresence());
    onRequestedPresenceChanged(account->requestedPresence());
    onChangingPresence(account->isChangingPresence());
    onConnectionStatusChanged(account->connectionStatus());

    if (m_hasEnabledAccounts != !m_enabledAccounts.data()->accounts().isEmpty()) {
        m_hasEnabledAccounts = !m_enabledAccounts.data()->accounts().isEmpty();
        Q_EMIT enabledAccountsChanged(m_hasEnabledAccounts);
    }

    qCDebug(KTP_COMMONINTERNALS) << "Account" << account->uniqueIdentifier()
      << "enabled:" << account->isEnabled();
}

void GlobalPresence::onCurrentPresenceChanged(const Tp::Presence &currentPresence)
{
    /* basic idea of choosing global presence it to make it reflects the presence
     * over all accounts, usually this is used to indicates user the whole system
     * status.
     *
     * If there isn't any account, currentPresence should be offline, since there is nothing
     * online.
     * If there's only one account, then currentPresence should represent the presence
     * of this account.
     * If there're more than one accounts, the situation is more complicated.
     * There can be some accounts is still connecting (thus it's offline), and there can be
     * some accounts doesn't support the presence you're choosing. The user-chosen presence
     * priority will be higher than standard presence order.
     *
     * Example:
     * user choose to be online, 1 account online, 1 account offline, current presence
     * should be online, since online priority is higher than offline.
     * user chooses a presence supported by part of the account, current presence will be
     * the one chosen by user, to indicate there is at least one account supports it.
     * user choose a presence supported by no account, current presence will be chosen
     * from all accounts based on priority, and it also indicates there is no account support
     * the user-chosen presence.
     */
    KTp::Presence highestCurrentPresence = KTp::Presence::offline();

    if (m_currentPresence == KTp::Presence(currentPresence)) {
        return;
    } else {
        for (const Tp::AccountPtr &account : m_enabledAccounts->accounts()) {
            if (KTp::Presence(account->currentPresence()) > highestCurrentPresence) {
                highestCurrentPresence = KTp::Presence(account->currentPresence());
            }
        }
    }

    if (m_currentPresence != highestCurrentPresence) {
        m_currentPresence = highestCurrentPresence;
        Q_EMIT currentPresenceChanged(m_currentPresence);
        qCDebug(KTP_COMMONINTERNALS) << "Current presence changed:"
          << m_currentPresence.status() << m_currentPresence.statusMessage();
    }
}

void GlobalPresence::onRequestedPresenceChanged(const Tp::Presence &requestedPresence)
{
    KTp::Presence highestRequestedPresence = KTp::Presence::offline();

    if (m_requestedPresence == KTp::Presence(requestedPresence)) {
        return;
    } else {
        for (const Tp::AccountPtr &account : m_enabledAccounts->accounts()) {
            if (KTp::Presence(account->requestedPresence()) > highestRequestedPresence) {
                highestRequestedPresence = KTp::Presence(account->requestedPresence());
            }
        }
    }

    if (m_requestedPresence != highestRequestedPresence) {
        m_requestedPresence = highestRequestedPresence;
        Q_EMIT requestedPresenceChanged(m_requestedPresence);
        qCDebug(KTP_COMMONINTERNALS) << "Requested presence changed:"
          << m_requestedPresence.status() << m_requestedPresence.statusMessage();
    }
}

void GlobalPresence::onChangingPresence(bool isChangingPresence)
{
    bool changing = false;

    if (m_changingPresence == isChangingPresence) {
        return;
    } else {
        for (const Tp::AccountPtr &account : m_enabledAccounts->accounts()) {
            changing = account->isChangingPresence();

            if (account->isChangingPresence()) {
                break;
            }
        }
    }

    if (m_changingPresence != changing) {
        m_changingPresence = changing;
        Q_EMIT changingPresence(m_changingPresence);
        qCDebug(KTP_COMMONINTERNALS) << "Presence changing:" << m_changingPresence;
    }
}

void GlobalPresence::onConnectionStatusChanged(Tp::ConnectionStatus connectionStatus)
{
    GlobalPresence::ConnectionStatus changedConnectionStatus = GlobalPresence::Disconnected;
    QList<GlobalPresence::ConnectionStatus> accountConnectionStatuses;
    bool hasConnectionError = false;

    if (m_connectionStatus == ConnectionStatus(connectionStatus)) {
        return;
    } else {
        for (const Tp::AccountPtr &account : m_enabledAccounts->accounts()) {
            accountConnectionStatuses << ConnectionStatus(account->connectionStatus());
            if (!account->connectionError().isEmpty()) {
                hasConnectionError = true;
            }
        }
    }

    if (accountConnectionStatuses.contains(GlobalPresence::Connecting)) {
        changedConnectionStatus = GlobalPresence::Connecting;
    } else if (accountConnectionStatuses.contains(GlobalPresence::Connected)) {
        changedConnectionStatus = GlobalPresence::Connected;
    }

    m_hasConnectionError = hasConnectionError;

    if (m_connectionStatus != changedConnectionStatus) {
        m_connectionStatus = changedConnectionStatus;
        Q_EMIT connectionStatusChanged(m_connectionStatus);
        qCDebug(KTP_COMMONINTERNALS) << "Connection status changed:" << m_connectionStatus;
    }
}

bool GlobalPresence::hasEnabledAccounts() const
{
    return m_hasEnabledAccounts;
}

bool GlobalPresence::hasConnectionError() const
{
    return m_hasConnectionError;
}

Tp::AccountSetPtr GlobalPresence::onlineAccounts() const
{
    return m_onlineAccounts;
}

Tp::AccountSetPtr GlobalPresence::enabledAccounts() const
{
    return m_enabledAccounts;
}

}


#include "global-presence.moc"
