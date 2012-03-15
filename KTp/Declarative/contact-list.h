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

#include <KTp/Models/accounts-model.h>
#include <KTp/Models/flat-model-proxy.h>
#include <KTp/Models/accounts-filter-model.h>


#include <TelepathyQt/Types>

/** Exposes general contact list stuff to QML*/
class ContactList : public QObject
{   
    Q_OBJECT
public:
    Q_PROPERTY(QObject* model READ flatModel)
    Q_PROPERTY(QObject* filter READ filterModel)

    
    ContactList(QObject *parent=0);
    FlatModelProxy* flatModel() const;
    AccountsFilterModel* filterModel() const;

    
public slots:
    void startChat(ContactModelItem *contact);

private slots:
    void onAccountManagerReady(Tp::PendingOperation *op);
    
private:
    AccountsModel* m_accountsModel;
    AccountsFilterModel* m_filterModel;
    FlatModelProxy* m_flatModel; 
    Tp::AccountManagerPtr m_accountManager;
};

#endif
