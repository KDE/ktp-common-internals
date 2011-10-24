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

#include <TelepathyQt4/AccountSet>
#include <TelepathyQt4/Account>

#include <KDebug>

GlobalPresence::GlobalPresence(QObject *parent)
    : QObject(parent),
      m_requestedPresence(Tp::Presence::offline()),
      m_currentPresence(Tp::Presence::offline()),
      m_changingPresence(false)
{
    m_presenceSorting[Tp::ConnectionPresenceTypeAvailable] = 0;
    m_presenceSorting[Tp::ConnectionPresenceTypeBusy] = 1;
    m_presenceSorting[Tp::ConnectionPresenceTypeHidden] = 2;
    m_presenceSorting[Tp::ConnectionPresenceTypeAway] = 3;
    m_presenceSorting[Tp::ConnectionPresenceTypeExtendedAway] = 4;
    //don't distinguish between the following three presences
    m_presenceSorting[Tp::ConnectionPresenceTypeError] = 5;
    m_presenceSorting[Tp::ConnectionPresenceTypeUnknown] = 5;
    m_presenceSorting[Tp::ConnectionPresenceTypeUnset] = 5;
    m_presenceSorting[Tp::ConnectionPresenceTypeOffline] = 6;

}

void GlobalPresence::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    if (! accountManager->isReady()) {
        kFatal("GlobalPresence used with unready account manager");
    }

    m_enabledAccounts = accountManager->enabledAccounts();

    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        onAccountAdded(account);
    }

    onCurrentPresenceChanged();
    onRequestedPresenceChanged();
    onChangingPresence();

    connect(m_enabledAccounts.data(), SIGNAL(accountAdded(Tp::AccountPtr)), SLOT(onAccountAdded(Tp::AccountPtr)));
}


Tp::Presence GlobalPresence::currentPresence() const
{
    return m_currentPresence;
}

Tp::Presence GlobalPresence::requestedPresence() const
{
    return m_requestedPresence;
}

bool GlobalPresence::isChangingPresence() const
{
    return m_changingPresence;
}


void GlobalPresence::setPresence(const Tp::Presence &presence)
{
    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        account->setRequestedPresence(presence);
    }
}


void GlobalPresence::onAccountAdded(const Tp::AccountPtr &account)
{
    connect(account.data(), SIGNAL(changingPresence(bool)), SLOT(onChangingPresence()));
    connect(account.data(), SIGNAL(requestedPresenceChanged(Tp::Presence)), SLOT(onRequestedPresenceChanged()));
    connect(account.data(), SIGNAL(currentPresenceChanged(Tp::Presence)), SLOT(onCurrentPresenceChanged()));
}

void GlobalPresence::onCurrentPresenceChanged()
{
    Tp::Presence highestCurrentPresence = Tp::Presence::offline();
    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        if (m_presenceSorting[account->currentPresence().type()] < m_presenceSorting[highestCurrentPresence.type()]) {
            highestCurrentPresence = account->currentPresence();
        }
    }

    qDebug() << "current presence changed";

    if (highestCurrentPresence.type() != m_currentPresence.type() ||
            highestCurrentPresence.status() != m_requestedPresence.status()) {
        m_currentPresence = highestCurrentPresence;
        qDebug() << "emit";
        Q_EMIT currentPresenceChanged(m_currentPresence);
    }
}

void GlobalPresence::onRequestedPresenceChanged()
{
    Tp::Presence highestRequestedPresence = Tp::Presence::offline();
    Q_FOREACH(const Tp::AccountPtr &account, m_enabledAccounts->accounts()) {
        if (m_presenceSorting[account->requestedPresence().type()] < m_presenceSorting[highestRequestedPresence.type()]) {
            highestRequestedPresence = account->currentPresence();
        }
    }

    if (highestRequestedPresence.type() != m_requestedPresence.type() &&
            highestRequestedPresence.status() != m_requestedPresence.status()) {
        m_requestedPresence = highestRequestedPresence;
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

#include "global-presence.moc"
