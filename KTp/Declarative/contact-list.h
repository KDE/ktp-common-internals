/*
    Copyright (C) 2011  David Edmundson <kde@davidedmundson.co.uk>

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

#ifndef CONTACT_LIST_H
#define CONTACT_LIST_H

#include <KTp/Models/contacts-list-model.h>
#include <KTp/Models/contacts-filter-model.h>
#include <KTp/contact.h>

#include <TelepathyQt/Types>

/** Exposes general contact list stuff to QML*/
class ContactList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* model READ filterModel CONSTANT)

  public:
    ContactList(QObject *parent=0);
    KTp::ContactsFilterModel* filterModel() const;

  public Q_SLOTS:
    void startChat(const Tp::AccountPtr &account, const KTp::ContactPtr &contact);

  private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onGenericOperationFinished(Tp::PendingOperation *op);

  private:
    KTp::ContactsListModel* m_contactsModel;
    KTp::ContactsFilterModel* m_filterModel;
    Tp::AccountManagerPtr m_accountManager;
};

#endif
