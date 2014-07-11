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

#include "proxy-service.h"
#include "proxy-observer.h"
#include "otr-proxy-channel.h"
#include "constants.h"

#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/PendingReady>

#include <KDebug>
#include <KApplication>

ProxyService::ProxyService(const QDBusConnection &dbusConnection)
    : Tp::DBusService(dbusConnection),
    adaptee(this, dbusConnection),
    observer(new ProxyObserver(this)),
    registrar(Tp::ClientRegistrar::create(dbusConnection))
{
}

ProxyService::~ProxyService()
{
    registrar->unregisterClients();
}

void ProxyService::addChannel(const Tp::ChannelPtr &channel)
{
    Tp::TextChannelPtr textChannel = Tp::TextChannel::create(channel->connection(), channel->objectPath(), QVariantMap());
    Tp::PendingReady *pendingReady =
        textChannel->becomeReady(Tp::Features()
            << Tp::TextChannel::FeatureCore
            << Tp::TextChannel::FeatureMessageQueue
            << Tp::TextChannel::FeatureMessageCapabilities
            );

    connect(pendingReady, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onChannelReady(Tp::PendingOperation*)));
}

void ProxyService::registerService(Tp::DBusError *error)
{
    Tp::AbstractClientPtr baseObserverPtr = Tp::AbstractClientPtr::dynamicCast(observer);
    const bool isRegistered = registrar->registerClient(baseObserverPtr, QString::fromLatin1("ProxyObserver"));
    if(!isRegistered) {
        error->set(
                QString::fromLatin1("Client registering error"),
                QString::fromLatin1("Could not register the observer"));
        return;
    }

    DBusService::registerObject(KTP_PROXY_BUS_NAME, KTP_PROXY_SERVICE_OBJECT_PATH, error);
}

QVariantMap ProxyService::immutableProperties() const
{
    return QVariantMap();
}

void ProxyService::onChannelProxyClosed()
{
    OtrProxyChannel *proxyChannel = dynamic_cast<OtrProxyChannel*>(QObject::sender());
    kDebug() << "Removing proxy: " << proxyChannel->objectPath()
        << " for the channel: " << proxyChannel->wrappedChannel()->objectPath();

    channels.remove(proxyChannel);
}

void ProxyService::onChannelReady(Tp::PendingOperation *pendingChanReady)
{
    if(pendingChanReady->isError()) {
        kWarning() << "Channel couldn't become ready due to: "
            << pendingChanReady->errorName() << " - " << pendingChanReady->errorMessage();
        return;
    }
    Tp::PendingReady *pendingReady = dynamic_cast<Tp::PendingReady*>(pendingChanReady);
    Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(pendingReady->proxy());
    kDebug() << "Channel ready: " << textChannel->objectPath();

    Tp::DBusError error;
    OtrProxyChannelPtr proxyChannel = OtrProxyChannel::create(dbusConnection(), textChannel);
    proxyChannel->registerService(&error);

    if(error.isValid()) {
        kError() << "Couldn't install proxy for the channel: " << textChannel->objectPath() << "\n"
            << "error name: " << error.name() << "\n"
            << "error message: " << error.message();

        return;
    }

    channels.insert(proxyChannel.data(), proxyChannel);

    connect(proxyChannel.data(), SIGNAL(closed()), SLOT(onChannelProxyClosed()));
    QObject::connect(
            proxyChannel.data(), SIGNAL(connected(const QDBusObjectPath&)),
            &adaptee, SLOT(onProxyConnected(const QDBusObjectPath&)));
    QObject::connect(
            proxyChannel.data(), SIGNAL(disconnected(const QDBusObjectPath&)),
            &adaptee, SLOT(onProxyDisconnected(const QDBusObjectPath&)));

    kDebug() << "Installed proxy: " << proxyChannel->objectPath() << "\n"
        << " for the channel: " << textChannel->objectPath();
}
