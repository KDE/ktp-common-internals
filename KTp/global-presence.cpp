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

#include "presence.h"

#include <TelepathyQt/AccountSet>
#include <TelepathyQt/Account>
#include <TelepathyQt/PendingReady>

#include <KDebug>

namespace KTp
{

GlobalPresence::GlobalPresence(QObject *parent)
    : QObject(parent),
      m_connectionStatus(Tp::ConnectionStatusDisconnected),
      m_changingPresence(false)
{
    Tp::Presence unknown;
    unknown.setStatus(Tp::ConnectionPresenceTypeUnknown, QLatin1String("unknown"), QString());

    m_requestedPresence = KTp::Presence(unknown);
    m_currentPresence = KTp::Presence(unknown);
}

void GlobalPresence::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    if (! accountManager->isReady()) {
        kWarning() << "GlobalPresence used with unready account manager";
    }

    m_enabledAccounts = accountManager->enabledAccounts();
    m_onlineAccounts = accountManager->onlineAccounts();

    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        onAccountAdded(account);
    }

    onCurrentPresenceChanged();
    onRequestedPresenceChanged();
    onChangingPresence();
    onConnectionStatusChanged();

    connect(m_enabledAccounts.data(), SIGNAL(accountAdded(Tp::AccountPtr)), SLOT(onAccountAdded(Tp::AccountPtr)));
}

void GlobalPresence::addAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    m_accountManager = accountManager;
    connect(m_accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

Tp::AccountManagerPtr GlobalPresence::accountManager() const
{
    return m_accountManager;
}

void GlobalPresence::onAccountManagerReady(Tp::PendingOperation* op)
{
    if (op->isError()) {
        kDebug() << op->errorName();
        kDebug() << op->errorMessage();

        //TODO: Create signal to send to client
        kDebug() << "Something unexpected happened to the core part of your Instant Messaging system "
                 << "and it couldn't be initialized. Try restarting the client.";

        return;
    }

    setAccountManager(m_accountManager);
    Q_EMIT(accountManagerReady());
}

Tp::ConnectionStatus GlobalPresence::connectionStatus() const
{
    return m_connectionStatus;
}

Presence GlobalPresence::currentPresence() const
{
    return m_currentPresence;
}

QString GlobalPresence::currentPresenceMessage() const
{
    KTp::Presence p = currentPresence();
    return p.statusMessage();
}

QIcon GlobalPresence::currentPresenceIcon() const
{
    return currentPresence().icon();
}

QString GlobalPresence::currentPresenceIconName() const
{
    return currentPresence().iconName();
}

GlobalPresence::ConnectionPresenceType GlobalPresence::currentPresenceType() const
{
    KTp::Presence p = currentPresence();
    switch(p.type()) {
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

Presence GlobalPresence::requestedPresence() const
{
    return m_requestedPresence;
}

bool GlobalPresence::isChangingPresence() const
{
    return m_changingPresence;
}

void GlobalPresence::setPresence(const KTp::Presence &presence)
{
    if (m_enabledAccounts.isNull()) {
        kWarning() << "Requested presence change on empty accounts set";
        return;
    }

    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        account->setRequestedPresence(presence);
    }
}

void GlobalPresence::setPresence(GlobalPresence::ConnectionPresenceType p, QString message)
{
    switch (p) {
    case GlobalPresence::Available:
        setPresence(Tp::Presence::available(message));
        break;
    case GlobalPresence::Busy:
        setPresence(Tp::Presence::busy(message));
        break;
    case GlobalPresence::Away:
        setPresence(Tp::Presence::away(message));
        break;
    case GlobalPresence::ExtendedAway:
        setPresence(Tp::Presence::xa(message));
        break;
    case GlobalPresence::Hidden:
        setPresence(Tp::Presence::hidden(message));
        break;
    case GlobalPresence::Offline:
        setPresence(Tp::Presence::offline(message));
        break;
    default:
        kDebug() << "You should not be here!";
    }
}

void GlobalPresence::onAccountAdded(const Tp::AccountPtr &account)
{
    connect(account.data(), SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)), SLOT(onConnectionStatusChanged()));
    connect(account.data(), SIGNAL(changingPresence(bool)), SLOT(onChangingPresence()));
    connect(account.data(), SIGNAL(requestedPresenceChanged(Tp::Presence)), SLOT(onRequestedPresenceChanged()));
    connect(account.data(), SIGNAL(currentPresenceChanged(Tp::Presence)), SLOT(onCurrentPresenceChanged()));
}

void GlobalPresence::onCurrentPresenceChanged()
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
    Tp::Presence highestCurrentPresence = Tp::Presence::offline();
    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        if (account->currentPresence().type() == m_requestedPresence.type()) {
            highestCurrentPresence = account->currentPresence();
            break;
        }

        if (Presence::sortPriority(account->currentPresence().type()) < Presence::sortPriority(highestCurrentPresence.type())) {
            highestCurrentPresence = account->currentPresence();
        }
    }

    kDebug() << "Current presence changed";

    if (highestCurrentPresence.type() != m_currentPresence.type() ||
            highestCurrentPresence.status() != m_currentPresence.status() ||
            highestCurrentPresence.statusMessage() != m_currentPresence.statusMessage()) {

        m_currentPresence = Presence(highestCurrentPresence);
        Q_EMIT currentPresenceChanged(m_currentPresence);
    }
}

void GlobalPresence::onRequestedPresenceChanged()
{
    Tp::Presence highestRequestedPresence = Tp::Presence::offline();
    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        if (Presence::sortPriority(account->requestedPresence().type()) < Presence::sortPriority(highestRequestedPresence.type())) {
            highestRequestedPresence = account->requestedPresence();
        }
    }

    if (highestRequestedPresence.type() != m_requestedPresence.type() ||
            highestRequestedPresence.status() != m_requestedPresence.status() ||
            highestRequestedPresence.statusMessage() != m_requestedPresence.statusMessage()) {
        m_requestedPresence = Presence(highestRequestedPresence);
        // current presence priority is affected by requested presence
        onCurrentPresenceChanged();
        Q_EMIT requestedPresenceChanged(m_requestedPresence);
    }
}

void GlobalPresence::onChangingPresence()
{
    bool isChangingPresence = false;
    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        if (account->isChangingPresence()) {
            isChangingPresence = true;
        }
    }

    if (isChangingPresence != m_changingPresence) {
        m_changingPresence = isChangingPresence;
        Q_EMIT changingPresence(m_changingPresence);
    }
}

void GlobalPresence::onConnectionStatusChanged()
{
    Tp::ConnectionStatus connectionStatus = Tp::ConnectionStatusDisconnected;

    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        switch (account->connectionStatus()) {
        case Tp::ConnectionStatusConnecting:
            //connecting is the highest state, use this always
            connectionStatus = Tp::ConnectionStatusConnecting;
            break;
        case Tp::ConnectionStatusConnected:
            //only set to connected if we're not at connecting
            if (connectionStatus == Tp::ConnectionStatusDisconnected) {
                connectionStatus = Tp::ConnectionStatusConnected;
            }
            break;
        default:
            break;
        }
    }

    if (connectionStatus != m_connectionStatus) {
        m_connectionStatus = connectionStatus;
        Q_EMIT connectionStatusChanged(m_connectionStatus);
    }
}


bool GlobalPresence::hasEnabledAccounts() const
{
    if (m_enabledAccounts->accounts().isEmpty()) {
        return false;
    }

    return true;
}

void GlobalPresence::saveCurrentPresence()
{
    kDebug() << "Saving presence with message:" << m_currentPresence.statusMessage();
    m_savedPresence = m_currentPresence;
}

void GlobalPresence::restoreSavedPresence()
{
    kDebug() << m_savedPresence.statusMessage();
    setPresence(m_savedPresence);
}

Tp::AccountSetPtr GlobalPresence::onlineAccounts() const
{
    return m_onlineAccounts;
}

}


#include "global-presence.moc"
