/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public		   *
 * License as published by the Free Software Foundation; either		   *
 * version 2.1 of the License, or (at your option) any later version.	   *
 * 									   *
 * This library is distributed in the hope that it will be useful,	   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU	   *
 * Lesser General Public License for more details.			   *
 * 									   *
 * You should have received a copy of the GNU Lesser General Public	   *
 * License along with this library; if not, write to the Free Software	   *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA*
 ***************************************************************************/

#include "otr-proxy-channel.h"
#include "otr-proxy-channel-adaptee.h"
#include "proxy-service.h"

#include "KTp/OTR/constants.h"

#include <TelepathyQt/Channel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/DBusError>
#include <TelepathyQt/Connection>

OtrProxyChannel::OtrProxyChannel(
        const QDBusConnection &dbusConnection,
        const Tp::TextChannelPtr &channel,
        const OTR::SessionContext &context,
        ProxyService *ps)
    : Tp::DBusService(dbusConnection),
    adaptee(new Adaptee(this, dbusConnection, channel, context, ps))
{
    connect(adaptee, SIGNAL(closed()), SLOT(onClosed()));
}

OtrProxyChannelPtr OtrProxyChannel::create(const QDBusConnection &dbusConnection, const Tp::TextChannelPtr &channel,
                const OTR::SessionContext &context, ProxyService *ps)
{
    return OtrProxyChannelPtr(new OtrProxyChannel(dbusConnection, channel, context, ps));
}

void OtrProxyChannel::registerService(Tp::DBusError *error)
{
    int index;
    QString connectionId = adaptee->channel()->connection()->objectPath();
    index = connectionId.lastIndexOf(QChar::fromLatin1('/'));
    connectionId = connectionId.mid(index+1);

    QString channelId = adaptee->channel()->objectPath();
    index = channelId.lastIndexOf(QChar::fromLatin1('/'));
    channelId = channelId.mid(index+1);

    QString objectPath = QString::fromLatin1("%1%2/%3").arg(KTP_PROXY_CHANNEL_OBJECT_PATH_PREFIX, connectionId, channelId);

    Tp::DBusService::registerObject(KTP_PROXY_BUS_NAME, objectPath, error);
}

QVariantMap OtrProxyChannel::immutableProperties() const
{
    return QVariantMap();
}

Tp::TextChannelPtr OtrProxyChannel::wrappedChannel() const
{
    return adaptee->channel();
}

void OtrProxyChannel::onClosed()
{
    Q_EMIT closed();
}

