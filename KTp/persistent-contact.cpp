/*
    Copyright (C) 2013 David Edmundson <davidedmundson@kde.org>

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

#include "persistent-contact.h"

#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ContactManager>


#include <KTp/core.h>

#include <KDebug>

namespace KTp
{
class PersistentContact::Private
{
public:
    QString contactId;
    QString accountId;
    KTp::ContactPtr contact;
    Tp::AccountPtr account;
};
}

KTp::PersistentContactPtr KTp::PersistentContact::create(const QString &accountId, const QString &contactId)
{
    return KTp::PersistentContactPtr(new KTp::PersistentContact(accountId, contactId));
}

KTp::PersistentContact::PersistentContact(const QString &accountId, const QString &contactId)
    : QObject(),
      d(new PersistentContact::Private())
{
    d->contactId = contactId;
    d->accountId = accountId;

    //FIXME there must be a const for this?
    QString objectPath = TP_QT_ACCOUNT_OBJECT_PATH_BASE + QLatin1Char('/') + accountId;

    Tp::PendingReady *op = KTp::accountFactory()->proxy(TP_QT_ACCOUNT_MANAGER_BUS_NAME, objectPath, KTp::connectionFactory(), KTp::channelFactory(), KTp::contactFactory());
    connect(op, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onCreateAccountFinished(Tp::PendingOperation*)));
}

KTp::PersistentContact::~PersistentContact()
{
    delete d;
}

QString KTp::PersistentContact::contactId() const
{
    return d->contactId;
}

QString KTp::PersistentContact::accountId() const
{
    return d->accountId;
}

void KTp::PersistentContact::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
}

KTp::ContactPtr KTp::PersistentContact::contact() const
{
    return d->contact;
}

Tp::AccountPtr KTp::PersistentContact::account() const
{
    return d->account;
}

void KTp::PersistentContact::onAccountReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "could not load account " << d->accountId;
    }
    Tp::PendingReady *pendingReady = qobject_cast<Tp::PendingReady*>(op);
    Q_ASSERT(pendingReady);
    Tp::AccountPtr account = Tp::AccountPtr::qObjectCast(pendingReady->proxy());
    d->account = account;
    connect(account.data(), SIGNAL(connectionChanged(Tp::ConnectionPtr)), SLOT(onAccountConnectionChanged(Tp::ConnectionPtr)));
    onAccountConnectionChanged(account->connection());
}


void KTp::PersistentContact::onAccountConnectionChanged(const Tp::ConnectionPtr &connection)
{
    if (connection) {
        Tp::ContactManagerPtr manager = connection->contactManager();
        connect(manager->contactsForIdentifiers(QStringList() << d->contactId), SIGNAL(finished(Tp::PendingOperation*)), SLOT(onPendingContactsFinished(Tp::PendingOperation*)));
    }
}

void KTp::PersistentContact::onPendingContactsFinished(Tp::PendingOperation *op)
{
    Tp::PendingContacts *pendingContactsOp = qobject_cast<Tp::PendingContacts*>(op);
    Q_ASSERT(pendingContactsOp);

    if (pendingContactsOp->contacts().size() == 1) {
        d->contact = KTp::ContactPtr::qObjectCast(pendingContactsOp->contacts()[0]);
        Q_EMIT contactChanged(d->contact);
        connect(d->contact.data(), SIGNAL(invalidated()), SLOT(onContactInvalid()));
    }
}

void KTp::PersistentContact::onContactInvalid()
{
    d->contact = KTp::ContactPtr();
    Q_EMIT contactChanged(d->contact);
}
