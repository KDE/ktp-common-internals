/*
    Copyright (C) 2013  Martin Klapetek <mklapetek@kde.org>
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


#include "im-persons-data-source.h"


#include <KPluginFactory>
#include <KPluginLoader>
#include <KDebug>

using namespace KPeople;

#include <TelepathyQt/Types>

#include "ktp-all-contacts-monitor.h"
#include "ktp-contact-monitor.h"

IMPersonsDataSource::IMPersonsDataSource(QObject *parent, const QVariantList &args)
    : BasePersonsDataSource(parent)
{
    Q_UNUSED(args);
    Tp::registerTypes();
}

IMPersonsDataSource::~IMPersonsDataSource()
{
}

QString IMPersonsDataSource::sourcePluginId() const
{
    return QLatin1String("ktp");
}

AllContactsMonitor* IMPersonsDataSource::createAllContactsMonitor()
{
    return new KTpAllContacts();
}

ContactMonitor* IMPersonsDataSource::createContactMonitor(const QString &contactId)
{
    return new KTpContactMonitor(contactId);
}



K_PLUGIN_FACTORY( IMPersonsDataSourceFactory, registerPlugin<IMPersonsDataSource>(); )
K_EXPORT_PLUGIN( IMPersonsDataSourceFactory("im_persons_data_source_plugin") )

#include "im-persons-data-source.moc"
