/*
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2012 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
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


#ifndef CONTACT_GRID_DELEGATE
#define CONTACT_GRID_DELEGATE

#include <QtGui/QWidget>
#include <QAbstractItemDelegate>
#include <TelepathyQt/Types>
#include <KTp/ktp-export.h>

class AccountsModel;
namespace KTp
{


class ContactGridDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactGridDelegate)

public:
    ContactGridDelegate(QObject *parent);
    virtual ~ContactGridDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    class Private;
    Private * const d;

}; // class ContactGridDelegate

class KTP_EXPORT ContactGridWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactGridWidget)

    Q_PRIVATE_SLOT(d, void _k_onSelectionChanged(QItemSelection,QItemSelection));

public:
   explicit ContactGridWidget(AccountsModel *model, QWidget *parent = 0);
   virtual ~ContactGridWidget();

    virtual bool hasSelection() const;
    virtual Tp::AccountPtr selectedAccount() const;
    virtual Tp::ContactPtr selectedContact() const;

    virtual void setShowOfflineUsers(bool showOfflineUsers);
    virtual void setFilterByFileTransferCapability(bool filterByFileTransferCapability);

    virtual void showNameFilterBar();
    virtual void hideNameFilterBar();
    virtual void setNameFilterBarShown(bool filterbarShown);
    virtual void setNameFilter(const QString &nameFilter);

Q_SIGNALS:
    void selectionChanged(Tp::AccountPtr selectedAccount, Tp::ContactPtr selectedContact);

private:
    class Private;
    Private * const d;

}; // class ContactGridWidget

} // namespace KTp

#endif // CONTACT_GRID_DELEGATE
