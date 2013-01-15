/*
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

#ifndef KTP_CONTACTS_LIST_MODEL_H
#define KTP_CONTACTS_LIST_MODEL_H

#include <QAbstractListModel>
#include <TelepathyQt/Types>

#include <KTp/global-contact-manager.h>
#include <KTp/ktp-export.h>

namespace KTp
{

class KTP_EXPORT ContactsListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ContactsListModel(QObject *parent = 0);
    virtual ~ContactsListModel();

    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

private Q_SLOTS:
    void onContactsChanged(const Tp::Contacts &added, const Tp::Contacts &removed);
    void onChanged();
    void onConnectionDropped();

private:
    Q_DISABLE_COPY(ContactsListModel)
    class Private;
    Private *d;

};

}
#endif // CONTACTSLISTMODEL_H
