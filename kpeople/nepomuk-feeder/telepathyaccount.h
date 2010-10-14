/*
 * This file is part of nepomuktelepathyservice
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
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

#ifndef TELEPATHY_INTEGRATION_DAEMON_TELEPATHYACCOUNT_H
#define TELEPATHY_INTEGRATION_DAEMON_TELEPATHYACCOUNT_H

#include "ontologies/imaccount.h"
#include "ontologies/personcontact.h"

#include <QtCore/QObject>
#include <QtCore/QString>

#include <TelepathyQt4/Account>
#include <TelepathyQt4/Connection>

class TelepathyContact;
class KJob;
class TelepathyAccountMonitor;

namespace Tp {
    class PendingOperation;
}

/**
 * This class takes care of one TelepathyAccount on the Account Manager.
 * It is responsible for keeping that account synced to Nepomuk, as well
 * as creating TelepathyContact objects for the contents of that account's
 * buddy list.
 */
class TelepathyAccount : public QObject
{
    Q_OBJECT

public:
    explicit TelepathyAccount(const QString &path, TelepathyAccountMonitor *parent = 0);
    ~TelepathyAccount();

    TelepathyAccountMonitor *monitor();

private Q_SLOTS:
    void onAccountReady(Tp::PendingOperation *op);
    void onHaveConnectionChanged(bool haveConnection);
    void onConnectionReady(Tp::PendingOperation *op);
    void onNicknameChanged(const QString &nickname);
    void onCurrentPresenceChanged(Tp::SimplePresence presence);
    void onContactsUpgraded(Tp::PendingOperation *op);
    void onAvatarChanged(const Tp::Avatar &avatar);
    void onContactAvatarRetrieved(uint contact, const QString &token, const QByteArray &avatar, const QString &mimetype);
    void onContactAvatarUpdated(uint contact, const QString &token);
    void onContactDestroyed(const Tp::ContactPtr &contact);
    void onAllKnownContactsChanged(const Tp::Contacts &added, const Tp::Contacts &removed);

private:
    Q_DISABLE_COPY(TelepathyAccount);

    void doNepomukSetup();
    Nepomuk::IMAccount getNepomukImAccount(const Nepomuk::PersonContact &mePersonContact);

    TelepathyAccountMonitor *m_parent;

    Tp::AccountPtr m_account;
    Nepomuk::IMAccount m_accountResource;
    Tp::ConnectionPtr m_connection;
    QString m_path;

    QHash< Tp::ContactPtr, TelepathyContact* > m_contacts;
};


#endif // Header guard

