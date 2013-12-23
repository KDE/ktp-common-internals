/*
* Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
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

#include "account-factory_p.h"

KTp::AccountFactory::AccountFactory(const QDBusConnection &bus, const Tp::Features &features):
    Tp::AccountFactory::AccountFactory(bus, features)
{
}

Tp::AccountFactoryPtr KTp::AccountFactory::create(const QDBusConnection &bus, const Tp::Features &features) {
    return Tp::AccountFactoryPtr(new KTp::AccountFactory(bus, features));
}

Tp::AccountPtr KTp::AccountFactory::construct(const QString &busName, const QString &objectPath, const Tp::ConnectionFactoryConstPtr &connFactory, const Tp::ChannelFactoryConstPtr &chanFactory, const Tp::ContactFactoryConstPtr &contactFactory) const
{
    return KTp::Account::create(QDBusConnection::sessionBus(), busName, objectPath, connFactory, chanFactory, contactFactory);
}



//account



Tp::AccountPtr KTp::Account::create(const QDBusConnection &bus, const QString &busName, const QString &objectPath, const Tp::ConnectionFactoryConstPtr &connectionFactory, const Tp::ChannelFactoryConstPtr &channelFactory, const Tp::ContactFactoryConstPtr &contactFactory)
{
    return Tp::AccountPtr(new KTp::Account(bus, busName, objectPath, connectionFactory, channelFactory, contactFactory, Tp::Account::FeatureCore));
}

KTp::Account::Account(const QDBusConnection &bus, const QString &busName, const QString &objectPath, const Tp::ConnectionFactoryConstPtr &connectionFactory, const Tp::ChannelFactoryConstPtr &channelFactory, const Tp::ContactFactoryConstPtr &contactFactory, const Tp::Feature &coreFeature):
    Tp::Account(bus, busName, objectPath, connectionFactory, channelFactory, contactFactory, coreFeature)
{
    connect(this, SIGNAL(connectionChanged(Tp::ConnectionPtr)), SLOT(onConnectionChanged(Tp::ConnectionPtr)));
}

void KTp::Account::onConnectionChanged(const Tp::ConnectionPtr &connection)
{
    if (connection) {
        connection->setProperty("accountUID", uniqueIdentifier());
    }
}
