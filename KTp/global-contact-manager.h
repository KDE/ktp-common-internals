/*
 * This file is part of telepathy-common-internals
 *
 * Copyright (C) 2012 David Edmundson <kde@davidedmundson.co.uk>
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

#ifndef GLOBALCONTACTMANAGER_H
#define GLOBALCONTACTMANAGER_H

#include <QObject>

#include <TelepathyQt/Types>
#include <TelepathyQt/ContactManager>

#include <KTp/ktp-export.h>

namespace KTp {

class GlobalContactManagerPrivate;

class KTP_EXPORT GlobalContactManager : public QObject
{
    Q_OBJECT
public:
    explicit GlobalContactManager(const Tp::AccountManagerPtr &accountManager, QObject *parent = 0);
    virtual ~GlobalContactManager();

    Tp::Contacts allKnownContacts() const;
    Tp::AccountPtr accountForConnection(const Tp::ConnectionPtr &connection) const;
    Tp::AccountPtr accountForContact(const Tp::ContactPtr &contact) const;
    Tp::AccountPtr accountForAccountId(const QString &accountId) const;

Q_SIGNALS:
    void allKnownContactsChanged(const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved);
    void presencePublicationRequested(const Tp::Contacts);

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onNewAccount(const Tp::AccountPtr &account);
    void onConnectionChanged(const Tp::ConnectionPtr &connection);
    void onContactManagerStateChanged(Tp::ContactListState state);

private:
    void onContactManagerStateChanged(const Tp::ContactManagerPtr &contactManager, Tp::ContactListState state);

    GlobalContactManagerPrivate *d;
};
}

#endif // GLOBALCONTACTLIST_H
