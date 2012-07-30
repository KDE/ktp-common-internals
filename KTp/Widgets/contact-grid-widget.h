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


#ifndef CONTACT_GRID_WIDGET_H
#define CONTACT_GRID_WIDGET_H

#include <QtGui/QWidget>
#include <QAbstractItemDelegate>
#include <TelepathyQt/Types>
#include <KTp/ktp-export.h>

class KLineEdit;
class AccountsModel;
class AccountsFilterModel;
namespace KTp
{

class KTP_EXPORT ContactGridWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactGridWidget)

    Q_PROPERTY(QString displayNameFilter
               READ displayNameFilter
               RESET clearDisplayNameFilter
               WRITE setDisplayNameFilter
               NOTIFY displayNameFilterChanged)
    Q_PROPERTY(QSize iconSize
               READ iconSize
               WRITE setIconSize
               NOTIFY iconSizeChanged)
public:
    explicit ContactGridWidget(AccountsModel *model, QWidget *parent = 0);
    virtual ~ContactGridWidget();

    virtual QString displayNameFilter() const;
    Q_SLOT virtual void clearDisplayNameFilter();
    Q_SLOT virtual void setDisplayNameFilter(const QString &displayNameFilter);
    Q_SIGNAL void displayNameFilterChanged(const QString &displayNameFilter);

    virtual QSize iconSize() const;
    Q_SLOT virtual void setIconSize(const QSize &iconSize);
    Q_SIGNAL void iconSizeChanged(const QSize &iconSize);

    virtual AccountsFilterModel* filter() const;
    virtual KLineEdit* contactFilterLineEdit() const;

    virtual bool hasSelection() const;
    virtual Tp::AccountPtr selectedAccount() const;
    virtual Tp::ContactPtr selectedContact() const;

Q_SIGNALS:
    void selectionChanged(Tp::AccountPtr selectedAccount, Tp::ContactPtr selectedContact);

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void _k_onSelectionChanged(QItemSelection,QItemSelection));

}; // class ContactGridWidget

} // namespace KTp

#endif // CONTACT_GRID_WIDGET_H
