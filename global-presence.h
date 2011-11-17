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

#ifndef GLOBALPRESENCE_H
#define GLOBALPRESENCE_H

#include <QObject>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/AccountSet>


/** This class handles the presence between all enabled accounts
 * It shows the highest current available presence, indicates if any accounts are changing, and what they are changing to.
*/

class GlobalPresence : public QObject
{
    Q_OBJECT
public:
    explicit GlobalPresence(QObject *parent = 0);

    /** Set the account manager to use
      * @param accountManager should be ready.
      */
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);


    /** The most online presence of any account*/
    Tp::Presence currentPresence() const;

    /** The most online presence requested for any account if any of the accounts are changing state.
      otherwise returns current presence*/
    Tp::Presence requestedPresence() const;

    /** Returns true if any account is changing state (i.e connecting*/
    bool isChangingPresence() const;

    /** Returns true if there is any enabled account */
    bool hasEnabledAccounts() const;

    Tp::AccountSetPtr onlineAccounts() const;

Q_SIGNALS:
    void requestedPresenceChanged(const Tp::Presence &customPresence);
    void currentPresenceChanged(const Tp::Presence &presence);
    void changingPresence(bool isChanging);

public Q_SLOTS:
    /** Set all enabled accounts to the specified presence*/
    void setPresence(const Tp::Presence &presence);

    /**Saves the current presence to memory*/
    void saveCurrentPresence();
    /**Restores the saved presence from memory */
    void restoreSavedPresence();

private Q_SLOTS:
    void onCurrentPresenceChanged();
    void onRequestedPresenceChanged();
    void onChangingPresence();

    void onAccountAdded(const Tp::AccountPtr &account);

private:
    Tp::AccountSetPtr m_enabledAccounts;
    Tp::AccountSetPtr m_onlineAccounts;

    /**Saved presence for later restoration (for example after returning from auto-away) */
    Tp::Presence m_savedPresence;
    /** A cache of the last sent requested presence, to avoid resignalling*/
    Tp::Presence m_requestedPresence;
    /** A cache of the last sent presence*/
    Tp::Presence m_currentPresence;
    bool m_changingPresence;
};

#endif // GLOBALPRESENCE_H
