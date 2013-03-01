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

#include "KTp/global-contact-manager.h"
#include "KTp/contact.h"

namespace KTp {

/** Object monitors a specific account/contact identifier and will populate it with the most up-to-date contact as connections get destroyed/created
 *
 */
class PersistentContact : public QObject
{
    Q_OBJECT
public:
    explicit PersistentContact(const QString &accountId, const QString contactId, KTp::GlobalContactManager *globalContactManager);

    QString contactId() const;
    QString accountId() const;

    /** The contact object for these ID
      @warning This may be null
    */
    KTp::ContactPtr contact() const;
    Tp::AccountPtr account() const;

Q_SIGNALS:
    /** Signals that the contact object has been replaced*/
    void contactChanged(KTp::ContactPtr);

private Q_SLOTS:
    void onAccountConnectionChanged(const Tp::ConnectionPtr &connection);
    void onPendingContactsFinished(Tp::PendingContacts*);

private:
    class Private;
    Private *d;
};

}
#endif // PERSISTENTCONTACT_H
