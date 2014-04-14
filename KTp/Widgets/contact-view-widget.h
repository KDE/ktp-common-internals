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


#ifndef CONTACT_VIEW_WIDGET_H
#define CONTACT_VIEW_WIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QListView>

#include <TelepathyQt/Types>

#include <KTp/contact.h>
#include <KTp/ktp-export.h>

class KLineEdit;
class QItemSelection;
class QListView;

namespace KTp
{
class ContactsFilterModel;
class ContactsListModel;

class KTP_EXPORT ContactViewWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactViewWidget)

    Q_PROPERTY(QString displayNameFilter
               READ displayNameFilter
               RESET clearDisplayNameFilter
               WRITE setDisplayNameFilter
               NOTIFY displayNameFilterChanged)
    Q_PROPERTY(QSize iconSize
               READ iconSize
               WRITE setIconSize
               NOTIFY iconSizeChanged)
    Q_PROPERTY(QListView::ViewMode viewMode
               READ viewMode
               WRITE setViewMode)
public:
    explicit ContactViewWidget(ContactsListModel *model, QWidget *parent = 0);
    virtual ~ContactViewWidget();

    virtual QString displayNameFilter() const;
    Q_SLOT virtual void clearDisplayNameFilter();
    Q_SLOT virtual void setDisplayNameFilter(const QString &displayNameFilter);
    Q_SIGNAL void displayNameFilterChanged(const QString &displayNameFilter);

    virtual QSize iconSize() const;
    Q_SLOT virtual void setIconSize(const QSize &iconSize);
    Q_SIGNAL void iconSizeChanged(const QSize &iconSize);

    virtual KTp::ContactsFilterModel* filter() const;
    virtual KLineEdit* contactFilterLineEdit() const;

    virtual bool hasSelection() const;
    virtual Tp::AccountPtr selectedAccount() const;
    virtual KTp::ContactPtr selectedContact() const;

    virtual QList<KTp::ContactPtr> selectedContacts() const;

    void setViewMode(QListView::ViewMode);
    QListView::ViewMode viewMode() const;

    void setSelectionMode(QAbstractItemView::SelectionMode mode);
    QAbstractItemView::SelectionMode selectionMode();

Q_SIGNALS:
    void selectionChanged(const Tp::AccountPtr &selectedAccount, const KTp::ContactPtr &selectedContact);
    void contactDoubleClicked(const Tp::AccountPtr &account, const KTp::ContactPtr &contact);

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void _k_onSelectionChanged(const QItemSelection &currentSelection,
                                                 const QItemSelection &previousSelection));
    Q_PRIVATE_SLOT(d, void _k_onDoubleClicked(const QModelIndex &index));

}; // class ContactViewWidget

} // namespace KTp

#endif // CONTACT_VIEW_WIDGET_H
