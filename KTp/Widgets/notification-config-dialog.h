/*
 * This file is part of KDE Telepathy Common Internals
 *
 * Copyright (C) 2012 Rohan Garg <rohangarg@kubuntu.org>
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

#ifndef NOTIFICATIONCONFIGDIALOG_H
#define NOTIFICATIONCONFIGDIALOG_H

#include <KTp/ktpcommoninternals_export.h>

#include <QDialog>
#include <TelepathyQt/Types>

class QDialogButtonBox;
class KNotifyConfigWidget;
class QAbstractButton;

namespace KTp {

class KTPCOMMONINTERNALS_EXPORT NotificationConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NotificationConfigDialog(const Tp::ContactPtr &contact, QWidget *parent = 0);
    ~NotificationConfigDialog();

private Q_SLOTS:
    void updateNotifyWidget(const int selection);
    void defaults();
    void onOkClicked();
    void onButtonBoxClicked(QAbstractButton *button);

private:
    KNotifyConfigWidget *m_notifyWidget;
    Tp::ContactPtr m_contact;
    int m_currentSelection;
    QDialogButtonBox *m_buttonBox;
};

}
#endif // NOTIFICATIONCONFIGDIALOG_H
