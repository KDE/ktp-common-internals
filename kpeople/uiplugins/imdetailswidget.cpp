/*
    Copyright (C) 2013  David Edmundson <davidedmundson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "imdetailswidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QApplication>
#include <QStyle>

#include <KLocalizedString>
#include <KPluginFactory>

#include <KPeople/PersonData>
#include <kpeople/personpluginmanager.h> //no pretty include exists at time of writing.

#include "KTp/im-persons-data-source.h"

K_PLUGIN_FACTORY( ImDetailsWidgetFactory, registerPlugin<ImDetailsWidget>(); )
K_EXPORT_PLUGIN( ImDetailsWidgetFactory("imdetailswidgetplugin", "ktp-common-internals"))

using namespace KPeople;

ImDetailsWidget::ImDetailsWidget(QWidget *parent, const QVariantList &args):
    AbstractPersonDetailsWidget(parent),
    m_layout(new QGridLayout(this))
{
    Q_UNUSED(args);
    setTitle(i18n("Instant Messaging Accounts"));
    setIcon(QIcon::fromTheme(QLatin1String("telepathy-kde")));

    setLayout(m_layout);
}

void ImDetailsWidget::setPerson(PersonData *person)
{
    const QStringList &imAccounts = person->imAccounts();

    //remove all existing widgets
    QLayoutItem *child;
    while ((child = m_layout->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }

    if (imAccounts.isEmpty()) {
        setActive(false);
        return;
    } else {
        setActive(true);
    }

    //fetch KTp::ContactPtr for the contact ID from KTp
    //display presence and address in grid
    IMPersonsDataSource *dataSource = dynamic_cast<IMPersonsDataSource*>(KPeople::PersonPluginManager::presencePlugin());
    for (int i=0; i<imAccounts.size(); i++) {
        const QString &contactId = imAccounts[i];
        KTp::ContactPtr contact = dataSource->contactForContactId(contactId);
        KTp::Presence presence;
        if (contact) {
            presence = contact->presence();
        } else {
            presence = KTp::Presence(Tp::Presence::offline());
        }

        QLabel *iconLabel = new QLabel(this);
        const int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
        iconLabel->setPixmap(presence.icon().pixmap(iconSize, iconSize));
        m_layout->addWidget(iconLabel, i, 0);

        QLabel *label = new QLabel(contactId, this);
        m_layout->addWidget(label, i, 1);
    }
}
