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

#include "otr-proxy-channel-adaptee.h"
#include "otr-proxy-channel.h"
#include "constants.h"
#include "pending-curry-operation.h"

#include <TelepathyQt/DBusObject>
#include <TelepathyQt/TextChannel>

#include <KDebug>

// TODO
// - add errors to spec

class SendMessageExtractor : public Extractor
{
    public:
        SendMessageExtractor()
            : token(QString::fromLatin1(""))
        { }

        void setContext(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr &context)
        {
            this->context = context;
        }

        Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr& getContext()
        {
            return context;
        }

        const QString& getToken() const
        {
            return token;
        }

        virtual void operator()(Tp::PendingOperation *op)
        {
            token = dynamic_cast<Tp::PendingSendMessage*>(op)->sentMessageToken();
        }

    private:
        QString token;
        Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr context;
};


OtrProxyChannel::Adaptee::Adaptee(OtrProxyChannel *pc, const QDBusConnection &dbusConnection, const Tp::TextChannelPtr &channel)
    : adaptor(new Tp::Service::ChannelProxyInterfaceOTRAdaptor(dbusConnection, this, pc->dbusObject())),
    pc(pc),
    chan(channel),
    isConnected(false)
{
    connect(chan.data(), SIGNAL(invalidated(Tp::DBusProxy*,const QString&,const QString&)), SIGNAL(closed()));
}

QDBusObjectPath OtrProxyChannel::Adaptee::wrappedChannel() const
{
    return QDBusObjectPath(chan->objectPath());
}

bool OtrProxyChannel::Adaptee::connected() const
{
    return isConnected;
}

Tp::MessagePartListList OtrProxyChannel::Adaptee::pendingMessages() const
{
    Tp::MessagePartListList messages;
    Q_FOREACH(const Tp::ReceivedMessage &mes, chan->messageQueue()) {
        messages << mes.parts();
    }

    return messages;
}

uint OtrProxyChannel::Adaptee::trustLevel() const
{
    return 0;
}

QString OtrProxyChannel::Adaptee::localFingerprint() const
{
    return QString::fromLatin1("not implemented");
}

QString OtrProxyChannel::Adaptee::remoteFingerprint() const
{
    return QString::fromLatin1("not implemented");
}

Tp::TextChannelPtr OtrProxyChannel::Adaptee::channel() const
{
    return chan;
}

void OtrProxyChannel::Adaptee::connectProxy(
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::ConnectProxyContextPtr &context)
{
    kDebug() << "Connecting proxy: " << pc->objectPath();
    connect(chan.data(),
            SIGNAL(messageReceived(const Tp::ReceivedMessage&)),
            SLOT(onMessageReceived(const Tp::ReceivedMessage&)));

    connect(chan.data(),
            SIGNAL(pendingMessageRemoved(const Tp::ReceivedMessage&)),
            SLOT(onPendingMessageRemoved(const Tp::ReceivedMessage&)));

    isConnected = true;
    context->setFinished();
}
void OtrProxyChannel::Adaptee::disconnectProxy(
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::DisconnectProxyContextPtr &context)
{
    kDebug() << "Disconnecting proxy: " << pc->objectPath();
    disconnect(chan.data(), SIGNAL(messageReceived(const Tp::ReceivedMessage&)),
            this, SLOT(onMessageReceived(const Tp::ReceivedMessage&)));

    disconnect(chan.data(), SIGNAL(pendingMessageRemoved(const Tp::ReceivedMessage&)),
            this, SLOT(onPendingMessageRemoved(const Tp::ReceivedMessage&)));

    isConnected = false;
    context->setFinished();
}

void OtrProxyChannel::Adaptee::sendMessage(const Tp::MessagePartList &message, uint flags,
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr &context)
{
    if(!connected()) {
        context->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }

    SendMessageExtractor *mesEx = new SendMessageExtractor();
    mesEx->setContext(context);
    PendingCurryOperation *pending = new PendingCurryOperation(
            chan->send(message, (Tp::MessageSendingFlags) flags),
            mesEx,
            Tp::SharedPtr<Tp::RefCounted>::dynamicCast(chan));

    connect(pending,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onPendingSendFinished(Tp::PendingOperation*)));
}

void OtrProxyChannel::Adaptee::acknowledgePendingMessages(const Tp::UIntList &ids,
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::AcknowledgePendingMessagesContextPtr &context)
{
    if(!connected()) {
        context->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }
    context->setFinished();
}

void OtrProxyChannel::Adaptee::initialize(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::InitializeContextPtr &context)
{
    if(!connected()) {
        context->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }
    context->setFinished();
}

void OtrProxyChannel::Adaptee::stop(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::StopContextPtr &context)
{
    if(!connected()) {
        context->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }
    context->setFinished();
}

void OtrProxyChannel::Adaptee::onMessageReceived(const Tp::ReceivedMessage &receivedMessage)
{
    kDebug() << "Received message: " << receivedMessage.text();
    emit messageReceived(receivedMessage.parts());
}

void OtrProxyChannel::Adaptee::onPendingMessageRemoved(const Tp::ReceivedMessage &receivedMessage)
{
    QDBusVariant variant = receivedMessage.header().value(QLatin1String("pending-message-id"));
    emit pendingMessagesRemoved(Tp::UIntList() << variant.variant().toUInt(NULL));
}


void OtrProxyChannel::Adaptee::onPendingSendFinished(Tp::PendingOperation *op)
{
    PendingCurryOperation *pendingSend = dynamic_cast<PendingCurryOperation*>(op);
    SendMessageExtractor &ex = dynamic_cast<SendMessageExtractor&>(pendingSend->extractor());

    Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr ctx = ex.getContext();

    if(op->isError()) {
        ctx->setFinishedWithError(op->errorName(), op->errorMessage());
    } else {
        ctx->setFinished(ex.getToken());
    }
}
