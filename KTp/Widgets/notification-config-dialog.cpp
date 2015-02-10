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

#include "notification-config-dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDBusInterface>
#include <QComboBox>

#include <TelepathyQt/Contact>

#include <KNotifyConfigWidget>
#include <KConfig>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>

KTp::NotificationConfigDialog::NotificationConfigDialog(const Tp::ContactPtr &contact, QWidget *parent)
    : QDialog(parent)
    , m_notifyWidget(new KNotifyConfigWidget(this))
{
    Q_ASSERT(contact);
    m_contact = contact;
    m_currentSelection = 0;
    setWindowTitle(i18n("Configure notifications for %1", m_contact.data()->alias()));
    setAttribute(Qt::WA_DeleteOnClose);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults, this);
    m_buttonBox->button(QDialogButtonBox::Apply)->setDisabled(true);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *vboxLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *hboxLayout = new QHBoxLayout(centralWidget);
    QLabel *label = new QLabel(i18n("Configure notifications for"), centralWidget);
    QComboBox *comboBox = new QComboBox(centralWidget);

    comboBox->setEditable(false);
    comboBox->addItem(m_contact.data()->alias());
    comboBox->addItem(i18n("All users"));
    hboxLayout->addWidget(label);
    hboxLayout->addWidget(comboBox);
    vboxLayout->addLayout(hboxLayout);
    vboxLayout->addWidget(m_notifyWidget);
    centralWidget->setLayout(vboxLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(centralWidget);
    mainLayout->addWidget(m_buttonBox);
    setLayout(mainLayout);

    m_notifyWidget->setApplication(QLatin1String("ktelepathy"),
                                   QLatin1String("contact"),
                                   m_contact.data()->id());

    connect(m_buttonBox, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(onButtonBoxClicked(QAbstractButton*)));
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(updateNotifyWidget(int)));

    connect(m_notifyWidget, &KNotifyConfigWidget::changed, [=](bool changed) {
        m_buttonBox->button(QDialogButtonBox::Apply)->setEnabled(changed);
    });
}

KTp::NotificationConfigDialog::~NotificationConfigDialog()
{
}

void KTp::NotificationConfigDialog::onButtonBoxClicked(QAbstractButton *button)
{
    switch (m_buttonBox->standardButton(button)) {
        case QDialogButtonBox::Ok:
            onOkClicked();
            break;
        case QDialogButtonBox::Apply:
            m_notifyWidget->save();
            break;
        case QDialogButtonBox::RestoreDefaults:
            defaults();
            break;
        case QDialogButtonBox::Cancel:
            reject();
            break;
        default:
            break;
    }
}

void KTp::NotificationConfigDialog::updateNotifyWidget(const int selection)
{
    if (selection == 0) {
        m_notifyWidget->setApplication(QLatin1String("ktelepathy"),
                                       QLatin1String("contact"),
                                       m_contact.data()->id());
        setWindowTitle(i18n("Configure notifications for %1", m_contact.data()->alias()));
    } else if (selection == 1) {
        m_notifyWidget->setApplication(QLatin1String("ktelepathy"));
        setWindowTitle(i18n("Configure notifications for all users"));
    }

    m_currentSelection = selection;
}

void KTp::NotificationConfigDialog::defaults()
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathy.notifyrc"));
    KConfigGroup *configGroup;

    if (m_currentSelection == 0) {
        Q_FOREACH(const QString &group, config->groupList()) {
            if (group.endsWith(m_contact.data()->id())) {
                configGroup = new KConfigGroup(config, group);
                configGroup->deleteGroup();
                delete configGroup;
            }
        }
    } else if (m_currentSelection == 1) {
        Q_FOREACH(const QString &group, config->groupList()) {
            if (group.startsWith(QLatin1String("Event"))) {
                configGroup = new KConfigGroup(config, group);
                configGroup->deleteGroup();
                delete configGroup;
            }
        }
    }
    config->sync();
    updateNotifyWidget(m_currentSelection);
}

void KTp::NotificationConfigDialog::onOkClicked()
{
    m_notifyWidget->save();
    accept();
}
