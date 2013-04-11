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
#include <QDBusInterface>

#include <TelepathyQt/Contact>

#include <KNotifyConfigWidget>
#include <KComboBox>
#include <KAboutData>
#include <KConfig>
#include <KSharedConfig>

KTp::NotificationConfigDialog::NotificationConfigDialog(const Tp::ContactPtr &contact, QWidget *parent)
    : KDialog(parent)
    , m_notifyWidget(new KNotifyConfigWidget(this))
{
    Q_ASSERT(contact);
    m_contact = contact;
    m_currentSelection = 0;
    setCaption(i18n("Configure notifications for %1", m_contact.data()->alias()));
    setAttribute(Qt::WA_DeleteOnClose);
    setButtons(KDialog::Ok | KDialog::Apply | KDialog::Cancel | KDialog::Default );
    enableButtonApply(false);

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

    connect(this, SIGNAL(okClicked()),
            SLOT(onOkClicked()));
    connect(this, SIGNAL(applyClicked()),
            SLOT(saveConfig()));
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(updateNotifyWidget(int)));
    connect(this, SIGNAL(defaultClicked()),
            SLOT(defaults()));
    connect(m_notifyWidget, SIGNAL(changed(bool)),
            SLOT(enableButtonApply(bool)));
}

KTp::NotificationConfigDialog::~NotificationConfigDialog()
{
}

void KTp::NotificationConfigDialog::saveConfig()
{
    m_notifyWidget->save();
}

void KTp::NotificationConfigDialog::updateNotifyWidget(const int selection)
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
    //ask the notify daemon to reload the config
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.knotify")))
    {
        QDBusInterface( QLatin1String("org.kde.knotify"), QLatin1String("/Notify"),
                        QLatin1String("org.kde.KNotify")).call( QLatin1String("reconfigure" ));
    }
    updateNotifyWidget(m_currentSelection);
}

void KTp::NotificationConfigDialog::onOkClicked()
{
    m_notifyWidget->save();
    accept();
}
