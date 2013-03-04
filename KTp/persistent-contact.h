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

#ifndef KTP_PERSISTENTCONTACT_H
#define KTP_PERSISTENTCONTACT_H

#include <QObject>

#include "KTp/contact.h"

#include "ktp-export.h"

namespace KTp {

/** Object monitors a specific account/contact identifier and will populate it with the most up-to-date contact as connections get destroyed/created
 *
 */
class KTP_EXPORT PersistentContact : public QObject, public Tp::RefCounted
{
    Q_OBJECT
public:
    static Tp::SharedPtr<KTp::PersistentContact> create(const QString &accountId, const QString &contactId);
    virtual ~PersistentContact();

    QString contactId() const;
    QString accountId() const;

    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    /** The contact object for these ID
      @warning This may be null if no accountManager is set or if you are offline
    */
    KTp::ContactPtr contact() const;
    /**
     * @warning This may be null if no accountManager is set or the account has been deleted
     */
    Tp::AccountPtr account() const;

Q_SIGNALS:
    /** Signals that the contact object has been replaced*/
    void contactChanged(KTp::ContactPtr);

private Q_SLOTS:
    void onAccountConnectionChanged(const Tp::ConnectionPtr &connection);
    void onPendingContactsFinished(Tp::PendingOperation*);
    void onContactInvalid();

private:
    PersistentContact(const QString &accountId, const QString &contactId);

    class Private;
    Private *d;
};

typedef Tp::SharedPtr<KTp::PersistentContact> PersistentContactPtr;

}
#endif // PERSISTENTCONTACT_H
