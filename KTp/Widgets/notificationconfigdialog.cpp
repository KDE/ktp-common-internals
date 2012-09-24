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

#include "notificationconfigdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include <TelepathyQt/Contact>

#include <KNotifyConfigWidget>
#include <KComboBox>
#include <KAboutData>

KTp::NotificationConfigDialog::NotificationConfigDialog(const Tp::ContactPtr &contact, QWidget *parent)
    : KDialog(parent)
    , m_notifyWidget(new KNotifyConfigWidget(this))
{
    Q_ASSERT(contact);
    m_contact = contact;
    setCaption(i18n("Configure notifications for %1", m_contact.data()->alias()));
    setAttribute(Qt::WA_DeleteOnClose);
    setButtons(KDialog::Apply | KDialog::Cancel);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *vboxLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *hboxLayout = new QHBoxLayout(centralWidget);
    QLabel *label = new QLabel(i18n("Configure notifications for"), centralWidget);
    KComboBox *comboBox = new KComboBox(centralWidget);

    comboBox->setEditable(false);
    comboBox->addItem(m_contact.data()->alias());
    comboBox->addItem(i18n("All users"));
    hboxLayout->addWidget(label);
    hboxLayout->addWidget(comboBox);
    vboxLayout->addLayout(hboxLayout);
    vboxLayout->addWidget(m_notifyWidget);
    centralWidget->setLayout(vboxLayout);
    setMainWidget(centralWidget);

    m_notifyWidget->setApplication(QLatin1String("ktelepathy"),
                                   QLatin1String("contact"),
                                   m_contact.data()->id());

    connect(this, SIGNAL(applyClicked()),
            SLOT(saveConfig()));
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(updateNotifyWidget(int)));
}

KTp::NotificationConfigDialog::~NotificationConfigDialog()
{
}

void KTp::NotificationConfigDialog::saveConfig()
{
    m_notifyWidget->save();
}

void KTp::NotificationConfigDialog::updateNotifyWidget(int selection)
{
    if (selection == 0) {
        m_notifyWidget->setApplication(QLatin1String("ktelepathy"),
                                       QLatin1String("contact"),
                                       m_contact.data()->id());
        setCaption(i18n("Configure notifications for %1", m_contact.data()->alias()));
    } else if (selection == 1) {
        m_notifyWidget->setApplication(QLatin1String("ktelepathy"));
        setCaption(i18n("Configure notifications for all users"));
    }
}

void KTp::NotificationConfigDialog::show()
{
    m_notifyWidget->show();
    setVisible(true);
}
