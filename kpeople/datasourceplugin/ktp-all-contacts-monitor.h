/*
 * Copyright (C) 2013  David Edmundson <davidedmundson@kde.org>
 * Copyright (C) 2013  Martin Klapetek <mklapetek@kde.org>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef KTPALLCONTACTSMONITOR_H
#define KTPALLCONTACTSMONITOR_H

#include <kpeople/allcontactsmonitor.h>
#include <TelepathyQt/Types>
#include <TelepathyQt/PendingOperation>
#include <KTp/types.h>

class KTpAllContacts : public KPeople::AllContactsMonitor
{
    Q_OBJECT
public:
    KTpAllContacts();
    ~KTpAllContacts();
    virtual KABC::Addressee::Map contacts();

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onContactChanged();
    void onContactInvalidated();
    void onAllKnownContactsChanged(const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved);

private:
    QString createUri(const KTp::ContactPtr &contact) const;
    KABC::Addressee contactToAddressee(const Tp::ContactPtr &contact) const;
    QHash<QString, KTp::ContactPtr> m_contacts;
};


#endif // KTPALLCONTACTSMONITOR_H
