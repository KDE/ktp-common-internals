/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2009-2011 Collabora Ltd. <info@collabora.co.uk>
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

#ifndef TELEPATHY_NEPOMUK_SERVICE_ACCOUNT_H
#define TELEPATHY_NEPOMUK_SERVICE_ACCOUNT_H

#include <QtCore/QObject>

#include <TelepathyQt/Account>
#include <TelepathyQt/Connection>

namespace Tp {
    class PendingOperation;
}

/**
 * This class takes care of one Telepathy Account on the Account Manager.
 * It is responsible for notifying the controller of changes to the Account, as well as
 * creating child Contact objects for each Telepathy Contact known by the Account.
 */
class Account : public QObject
{
    Q_OBJECT

public:
    explicit Account(const Tp::AccountPtr &account, QObject *parent = 0);
    ~Account();

    void init();
    void shutdown();

Q_SIGNALS:
    void created(const QString &path, const QString &id, const QString &protocol);
    void accountDestroyed(const QString &path);
    void nicknameChanged(const QString &path, const QString &nickname);
    void initialContactsLoaded(const QString &path, const Tp::Contacts &contacts);
    void accountRemoved(const QString &path);

    void contactCreated(const QString &path, const Tp::ContactPtr &contact);
    void contactRemoved(const QString &path, const Tp::ContactPtr &contact);
    void contactAliasChanged(const QString &path, const QString &id, const QString &alias);
    void contactGroupsChanged(const QString &path, const QString &id, const QStringList &groups);
    void contactAvatarChanged(const QString &path, const QString &id, const Tp::AvatarData &avatar);

private Q_SLOTS:
    void onConnectionChanged(const Tp::ConnectionPtr &connection);
    void onContactManagerStateChanged(Tp::ContactListState state);
    void onNicknameChanged(const QString &nickname);
    void onAllKnownContactsChanged(const Tp::Contacts &added, const Tp::Contacts &removed);
    void onNewContact(const Tp::ContactPtr &contact);
    void onContactRemoved(const Tp::ContactPtr &contact);
    void onAccountRemoved();

    void onContactAddedToGroup();
    void onContactAddedToGroup(const Tp::ContactPtr &contact);
    void onContactRemovedFromGroup(const QString &group);
    void onContactAliasChanged();
    void onContactAliasChanged(const Tp::ContactPtr &contact);
    void onContactAvatarChanged(const Tp::AvatarData &avatar);

private:
    Q_DISABLE_COPY(Account);

    void loadContacts();

    Tp::AccountPtr m_account;
    Tp::ConnectionPtr m_connection;
    QList< Tp::ContactPtr > m_contacts;
};

#endif // Header guard
