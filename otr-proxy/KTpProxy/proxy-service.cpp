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
#include "otr-config.h"
#include "otr-utils.h"
#include "pending-curry-operation.h"
#include "constants.h"

#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/PendingReady>

#include <KDebug>
#include <KApplication>

class PendingChannelReadyResult : public PendingCurryOperation
{
    public:
        PendingChannelReadyResult(Tp::PendingOperation *op, const Tp::TextChannelPtr &chan, const Tp::AccountPtr &account)
            : PendingCurryOperation(op, Tp::SharedPtr<Tp::RefCounted>::dynamicCast(chan)),
            channel(chan),
            account(account)
        {
        }

        virtual void extract(Tp::PendingOperation *op)
        {
            Q_UNUSED(op);
        }

        Tp::TextChannelPtr channel;
        Tp::AccountPtr account;
};

ProxyService::ProxyService(const QDBusConnection &dbusConnection, OTR::Config *config)
    : Tp::DBusService(dbusConnection),
    adaptee(this, dbusConnection),
    observer(new ProxyObserver(this)),
    registrar(Tp::ClientRegistrar::create(dbusConnection)),
    manager(config),
    am(Tp::AccountManager::create(dbusConnection))
{
}

ProxyService::~ProxyService()
{
    registrar->unregisterClients();
}

OTR::Manager* ProxyService::managerOTR()
{
    return &manager;
}

Tp::AccountManagerPtr ProxyService::accountManager()
{
    return am;
}

void ProxyService::addChannel(const Tp::ChannelPtr &channel, const Tp::AccountPtr &account)
{
    Tp::TextChannelPtr textChannel = Tp::TextChannel::create(channel->connection(), channel->objectPath(), QVariantMap());

    PendingChannelReadyResult *pending = new PendingChannelReadyResult(
            textChannel->becomeReady(Tp::Features()
                << Tp::TextChannel::FeatureCore
                << Tp::TextChannel::FeatureMessageQueue
                << Tp::TextChannel::FeatureMessageCapabilities
                << Tp::TextChannel::FeatureChatState
                ),
            textChannel,
            account);


    connect(pending, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onChannelReady(Tp::PendingOperation*)));
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

OtrlPolicy ProxyService::getPolicy() const
{
    return manager.getPolicy();
}

void ProxyService::setPolicy(OtrlPolicy otrPolicy)
{
    manager.setPolicy(otrPolicy);
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
    PendingChannelReadyResult *pendingReady = dynamic_cast<PendingChannelReadyResult*>(pendingChanReady);
    Tp::TextChannelPtr textChannel = pendingReady->channel;
    kDebug() << "Channel ready: " << textChannel->objectPath();

    Tp::DBusError error;
    OTR::SessionContext ctx =
    {
        OTR::utils::accountIdFor(QDBusObjectPath(pendingReady->account->objectPath())),
        pendingReady->account->normalizedName(),
        textChannel->targetId(),
        textChannel->connection()->protocolName()
    };

    OtrProxyChannelPtr proxyChannel = OtrProxyChannel::create(dbusConnection(), textChannel, ctx, this);
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

bool ProxyService::createNewPrivateKey(const QString &accountId, const QString &accountName)
{
    kDebug() << "Generating new private key for " << accountId;
    OTR::KeyGenerationThread *keyThread = manager.createNewPrivateKey(accountId, accountName);
    if(keyThread) {
        if(keyThread->prepareCreation()) {
            // alredy started creation for this account
            kDebug() << "Cannot prepare key generation for " << accountId;
            return false;
        }
        connect(keyThread, SIGNAL(finished()), SLOT(onKeyGenerationThreadFinished()));
        keyThread->start();
        Q_EMIT keyGenerationStarted(accountId);
        return true;
    } else {
        return false;
    }
}

void ProxyService::onKeyGenerationThreadFinished()
{
    OTR::KeyGenerationThread *thread = dynamic_cast<OTR::KeyGenerationThread*>(QObject::sender());
    kDebug() << "Finished generating a new private key for " << thread->accountId;

    if(thread->error() || thread->finalizeCreation()) {
        kDebug() << "Error generating private key for " << thread->accountId;
        Q_EMIT keyGenerationFinished(thread->accountId, true);
    } else{
        Q_EMIT keyGenerationFinished(thread->accountId, false);
    }

    thread->deleteLater();
}
