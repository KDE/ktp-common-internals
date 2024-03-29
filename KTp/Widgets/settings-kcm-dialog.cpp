/*
 * Settings KCM Dialog
 *
 * Copyright (C) 2011 Martin Klapetek <martin.klapetek@gmail.com>
 * Copyright (C) 2014 Siddhartha Sahu <sh.siddhartha@gmail.com>
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

#include "settings-kcm-dialog.h"

#include <KNotifyConfigWidget>
#include <KMessageBox>
#include <KLocalizedString>

namespace KTp
{

SettingsKcmDialog::SettingsKcmDialog(QWidget *parent) :
    KSettings::Dialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(700, 640);

    
    if (KPluginMetaData data(QStringLiteral("plasma/kcms/systemsettings/kcm_kaccounts")); data.isValid()) {
        addModule(data);
    } else if (KService::serviceByDesktopName(QStringLiteral("kcm_kaccounts"))) {
        addModule(QStringLiteral("kcm_kaccounts"));
    } else {
        KMessageBox::error(this,
            i18n("It appears you do not have the IM Accounts control module installed. Please install kaccounts-integration package."),
            i18n("IM Accounts KCM Plugin Is Not Installed"));

    }

}

void SettingsKcmDialog::addGeneralSettingsModule()
{
    addModule(QLatin1String("kcm_ktp_integration_module"));
}

void SettingsKcmDialog::addNotificationsModule()
{
    KNotifyConfigWidget *notificationWidget = new KNotifyConfigWidget(this);
    notificationWidget->setApplication(QLatin1String("ktelepathy"));
    connect(this, SIGNAL(accepted()),
            notificationWidget, SLOT(save()));

    connect(notificationWidget, SIGNAL(changed(bool)),
            this, SLOT(enableButtonApply(bool)));

    connect(this, SIGNAL(applyClicked()),
            notificationWidget, SLOT(save()));

    KPageWidgetItem *notificationPage = new KPageWidgetItem(notificationWidget, i18n("Notifications"));
    notificationPage->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-notification")));
    addPage(notificationPage);
}

}
