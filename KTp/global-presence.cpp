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

#include <KDebug>

namespace KTp
{

GlobalPresence::GlobalPresence(QObject *parent)
    : QObject(parent),
      m_requestedPresence(Presence(Tp::Presence::offline())),
      m_currentPresence(Presence(Tp::Presence::offline())),
      m_changingPresence(false)
{
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

    connect(m_enabledAccounts.data(), SIGNAL(accountAdded(Tp::AccountPtr)), SLOT(onAccountAdded(Tp::AccountPtr)));
}


Tp::ConnectionStatus GlobalPresence::connectionStatus() const
{
    return m_connectionStatus;
}

Presence GlobalPresence::currentPresence() const
{
    return m_currentPresence;
}

Presence GlobalPresence::requestedPresence() const
{
    return m_requestedPresence;
}

bool GlobalPresence::isChangingPresence() const
{
    return m_changingPresence;
}

void GlobalPresence::setPresence(const Tp::Presence &presence)
{
    if (m_enabledAccounts.isNull()) {
        kWarning() << "Requested presence change on empty accounts set";
        return;
    }

    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        account->setRequestedPresence(presence);
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
    Tp::Presence highestCurrentPresence = Tp::Presence::offline();
    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
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
