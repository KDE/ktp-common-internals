/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "otr-proxy-channel.h"
#include "otr-proxy-channel-adaptee.h"
#include "constants.h"

#include <TelepathyQt/Channel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/DBusError>
#include <TelepathyQt/Connection>

class OtrProxyChannel::Private
{
    public:
        Private(const QDBusConnection &dbusConnection,
                OtrProxyChannel *pc,
                const Tp::TextChannelPtr &channel,
                const OTR::SessionContext &context,
                OTR::Manager *manager)
            : adaptee(pc, dbusConnection, channel, context, manager)
        {
        }

        Adaptee adaptee;
};

OtrProxyChannel::OtrProxyChannel(
        const QDBusConnection &dbusConnection,
        const Tp::TextChannelPtr &channel,
        const OTR::SessionContext &context,
        OTR::Manager *manager)
    : Tp::DBusService(dbusConnection),
    d(new Private(dbusConnection, this, channel, context, manager))
{
    connect(&d->adaptee, SIGNAL(closed()), SLOT(onClosed()));
}

OtrProxyChannel::~OtrProxyChannel()
{
    delete d;
}

OtrProxyChannelPtr OtrProxyChannel::create(const QDBusConnection &dbusConnection, const Tp::TextChannelPtr &channel,
                const OTR::SessionContext &context, OTR::Manager *manager)
{
    return OtrProxyChannelPtr(new OtrProxyChannel(dbusConnection, channel, context, manager));
}

void OtrProxyChannel::registerService(Tp::DBusError *error)
{
    int index;
    QString connectionId = d->adaptee.channel()->connection()->objectPath();
    index = connectionId.lastIndexOf(QChar::fromLatin1('/'));
    connectionId = connectionId.mid(index+1);

    QString channelId = d->adaptee.channel()->objectPath();
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
    return d->adaptee.channel();
}

void OtrProxyChannel::onClosed()
{
    emit closed();
}

