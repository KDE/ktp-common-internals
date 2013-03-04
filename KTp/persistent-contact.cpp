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
#include <TelepathyQt/ContactManager>

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
    Q_FOREACH(const Tp::AccountPtr &account, accountManager->allAccounts()) {
        if (account->uniqueIdentifier() == d->accountId) {
            d->account = account;
            connect(account.data(), SIGNAL(connectionChanged(Tp::ConnectionPtr)), SLOT(onAccountConnectionChanged(Tp::ConnectionPtr)));
            onAccountConnectionChanged(account->connection());
            return;
        }
    }
    kWarning() << "Could not find account " << d->accountId;
}

KTp::ContactPtr KTp::PersistentContact::contact() const
{
    return d->contact;
}

Tp::AccountPtr KTp::PersistentContact::account() const
{
    return d->account;
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
