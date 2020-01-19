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
#include <QIcon>
#include <QDBusInterface>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/Constants>

#include <KTp/ktpcommoninternals_export.h>
#include <KTp/types.h>
#include <KTp/presence.h>

namespace KTp
{

/** This class handles the presence between all enabled accounts.
 * It shows the highest current / requested presence, indicates if any accounts
 * are changing state, and the highest they are changing to.
*/

class KTPCOMMONINTERNALS_EXPORT GlobalPresence : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Tp::AccountManagerPtr accountManager READ accountManager WRITE addAccountManager NOTIFY accountManagerReady)

    Q_PROPERTY(QString presenceMessage READ currentPresenceMessage NOTIFY currentPresenceChanged)
    Q_PROPERTY(ConnectionPresenceType presenceType READ currentPresenceType NOTIFY currentPresenceChanged)
    Q_PROPERTY(QIcon currentPresenceIcon READ currentPresenceIcon NOTIFY currentPresenceChanged)
    Q_PROPERTY(QString currentPresenceIconName READ currentPresenceIconName NOTIFY currentPresenceChanged)
    Q_PROPERTY(KTp::Presence currentPresence READ currentPresence NOTIFY currentPresenceChanged)
    Q_PROPERTY(QString currentPresenceName READ currentPresenceName NOTIFY currentPresenceChanged);
    Q_PROPERTY(KTp::Presence requestedPresence READ requestedPresence NOTIFY requestedPresenceChanged WRITE setPresence)
    Q_PROPERTY(QString requestedPresenceName READ requestedPresenceName NOTIFY requestedPresenceChanged)
    Q_PROPERTY(KTp::Presence globalPresence READ globalPresence WRITE setPresence)
    Q_PROPERTY(ConnectionStatus connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool isChangingPresence READ isChangingPresence NOTIFY changingPresence)
    Q_PROPERTY(bool hasConnectionError READ hasConnectionError NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool hasEnabledAccounts READ hasEnabledAccounts NOTIFY enabledAccountsChanged)
    Q_PROPERTY(Tp::AccountSetPtr enabledAccounts READ enabledAccounts)
    Q_PROPERTY(Tp::AccountSetPtr onlineAccounts READ onlineAccounts)

public:
    explicit GlobalPresence(QObject *parent = nullptr);

    enum ConnectionPresenceType
    {
        Offline = Tp::ConnectionPresenceTypeOffline,
        Available = Tp::ConnectionPresenceTypeAvailable,
        Away = Tp::ConnectionPresenceTypeAway,
        ExtendedAway = Tp::ConnectionPresenceTypeExtendedAway,
        Hidden = Tp::ConnectionPresenceTypeHidden,
        Busy = Tp::ConnectionPresenceTypeBusy,
        Unknown = Tp::ConnectionPresenceTypeUnknown,
        Unset = Tp::ConnectionPresenceTypeUnset,
        Error = Tp::ConnectionPresenceTypeError
    };
    Q_ENUM(ConnectionPresenceType)

    enum ConnectionStatus
    {
        Disconnected = Tp::ConnectionStatusDisconnected,
        Connecting = Tp::ConnectionStatusConnecting,
        Connected = Tp::ConnectionStatusConnected
    };
    Q_ENUM(ConnectionStatus)

    enum PresenceClass
    {
        Persistent,
        Session
    };
    Q_ENUM(PresenceClass)

    /**
     * \brief Set a ready account manager.
     *
     * \param accountManager A Tp::AccountManagerPtr.
     */
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    /**
     * \brief Add a new (unready) account manager.
     *
     * \param accountManager A Tp::AccountManagerPtr.
     */
    void addAccountManager(const Tp::AccountManagerPtr &accountManager);

    /**
     * \brief The account manager.
     *
     * \return A Tp::AccountManagerPtr.
     */
    Tp::AccountManagerPtr accountManager() const;

    /**
     * \brief Global connection status. Returns connecting if any account is
     * connecting, else connected if at least one account is connected,
     * disconnected otherwise.
     *
     * \return A ConnectionStatus enum.
     */
    ConnectionStatus connectionStatus() const;

    /**
     * \brief The most online presence of all accounts. Returns the same presence
     * as the requested presence if the most online account supports the
     * requested presence.
     */
    KTp::Presence currentPresence() const;
    QString currentPresenceMessage() const;
    QIcon currentPresenceIcon() const;
    QString currentPresenceIconName() const;
    ConnectionPresenceType currentPresenceType() const;
    QString currentPresenceName() const;

    /**
     * \brief The most online requested presence for all accounts.
     */
    KTp::Presence requestedPresence() const;
    QString requestedPresenceName() const;

    /**
     * \brief If any account is changing presence.
     *
     * \return true if any account is changing state.
     */
    bool isChangingPresence() const;

    /**
     * \brief The KDED module requested global presence.
     *
     * \return A KTp::Presence.
     */
    KTp::Presence globalPresence() const;

    /**
     * \brief If any account has a connection error.
     *
     * \return true if any account has a connection error.
     */
    bool hasConnectionError() const;

    /**
     * \brief If the account manager has enabled accounts.
     *
     * \return true if the account manager has enabled accounts.
     */
    bool hasEnabledAccounts() const;

    /**
     * \brief The account manager enabled accounts.
     *
     * \return The account manager enabled accounts set.
     */
    Tp::AccountSetPtr enabledAccounts() const;

    /**
     * \brief The account manager online accounts.
     *
     * \return The account manager online accounts set.
     */
    Tp::AccountSetPtr onlineAccounts() const;

Q_SIGNALS:
    void requestedPresenceChanged(const KTp::Presence &requestedPresence);
    void currentPresenceChanged(const KTp::Presence &currentPresence);
    void connectionStatusChanged(KTp::GlobalPresence::ConnectionStatus connectionStatus);
    void changingPresence(bool isChangingPresence);
    void enabledAccountsChanged(bool hasEnabledAccounts);
    void accountManagerReady();

public Q_SLOTS:
    /**
     * \brief Set the requested presence of all enabled accounts. If setting
     * the global requested presence fails, will set each account to the
     * specified presence. A presence type of unset will unset the presence.
     *
     * \param presence The requested presence.
     *
     * \overload presenceClass Session or Persistent presence class.
     **/
    void setPresence(const KTp::Presence &presence, PresenceClass presenceClass = Persistent);

    /**
     * \brief Set the requested presence of all enabled accounts. If setting
     * the global requested presence fails, will set each account to the
     * specified presence. A presence type of unset will unset the presence.
     *
     * \param type The ConnectionPresenceType.
     *
     * \overload message A status message.
     * \overload presenceClass Session or Persistent presence class.
     **/
    void setPresence(ConnectionPresenceType type, QString message = QString(), PresenceClass presenceClass = Session);

private Q_SLOTS:
    void onCurrentPresenceChanged(const Tp::Presence &currentPresence);
    void onRequestedPresenceChanged(const Tp::Presence &requestedPresence);
    void onChangingPresence(bool isChangingPresence);
    void onConnectionStatusChanged(Tp::ConnectionStatus connectionStatus);

    void onAccountEnabledChanged(const Tp::AccountPtr &account);

private:
    QDBusInterface *m_statusHandlerInterface;
    Tp::AccountManagerPtr m_accountManager;
    Tp::AccountSetPtr m_enabledAccounts;
    Tp::AccountSetPtr m_onlineAccounts;

    KTp::Presence m_requestedPresence;
    KTp::Presence m_currentPresence;
    ConnectionStatus m_connectionStatus;
    bool m_changingPresence;
    bool m_hasConnectionError;
    bool m_hasEnabledAccounts;
};

}

#endif // GLOBALPRESENCE_H
