/*
 * Add contact dialog
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2012 George Kiagiadakis <kiagiadakis.george@gmail.com>
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

#ifndef ADDCONTACTDIALOG_H
#define ADDCONTACTDIALOG_H

#include <KDialog>

#include <TelepathyQt/Types>

#include <KTp/ktp-export.h>

namespace Tp {
    class PendingOperation;
}

namespace KTp
{
class KTP_EXPORT AddContactDialog : public KDialog
{
    Q_OBJECT

public:
    explicit AddContactDialog(const Tp::AccountManagerPtr &accountManager, QWidget *parent = 0);
    virtual ~AddContactDialog();

    virtual void accept();

protected:
    virtual void closeEvent(QCloseEvent *e);

private Q_SLOTS:
    KTP_NO_EXPORT void _k_onContactsForIdentifiersFinished(Tp::PendingOperation *op);
    KTP_NO_EXPORT void _k_onRequestPresenceSubscriptionFinished(Tp::PendingOperation *op);
    KTP_NO_EXPORT void _k_onAccountUpgraded(Tp::PendingOperation *op);
    void updateSubscriptionMessageVisibility();

private:
    KTP_NO_EXPORT void setInProgress(bool inProgress);

    struct Private;
    Private * const d;
};
}

#endif // ADDCONTACTDIALOG_H
