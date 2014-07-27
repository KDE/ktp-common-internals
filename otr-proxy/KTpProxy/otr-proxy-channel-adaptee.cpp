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

class PendingSendMessageResult : public PendingCurryOperation
{
    public:
        PendingSendMessageResult(Tp::PendingOperation *op,
                const Tp::TextChannelPtr &chan,
                const Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr &context,
                const Tp::MessagePartList &message,
                uint flags)
            : PendingCurryOperation(op, Tp::SharedPtr<Tp::RefCounted>::dynamicCast(chan)),
            token(QString::fromLatin1("")),
            context(context),
            message(message),
            flags(flags)
        { }

        Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr getContext()
        {
            return context;
        }

        uint getFlags()
        {
            return flags;
        }

        Tp::MessagePartList getMessage()
        {
            return message;
        }

        const QString& getToken() const
        {
            return token;
        }

        virtual void extract(Tp::PendingOperation *op)
        {
            token = dynamic_cast<Tp::PendingSendMessage*>(op)->sentMessageToken();
        }

    private:
        QString token;
        Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr context;
        Tp::MessagePartList message;
        uint flags;
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
    Tp::MessagePartListList pending;
    for(const auto& mp: messages) {
        pending << mp.parts();
    }

    return pending;
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

    for(const Tp::ReceivedMessage &m: chan->messageQueue()) {
        onMessageReceived(m);
    }

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
    messages.clear();
    context->setFinished();
}

void OtrProxyChannel::Adaptee::sendMessage(const Tp::MessagePartList &message, uint flags,
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr &context)
{
    if(!connected()) {
        context->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }

    kDebug();
    PendingSendMessageResult *pending = new PendingSendMessageResult(
            chan->send(message, (Tp::MessageSendingFlags) flags),
            chan,
            context,
            message,
            flags);

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

    kDebug() << "Message queue size: " << messages.size();
    QList<Tp::ReceivedMessage> toAcknowledge;
    for(uint id: ids) {
        auto found = messages.find(id);
        if(found == messages.end()) {
            kDebug() << "Client trying to acknowledge non existing message with id" << id;
            context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT,
                    QLatin1String("Message with given ID is not present in the message queue"));
            return;
        } else {
            toAcknowledge << *found;
        }
    }

    kDebug() << "Acknowledging " << toAcknowledge.count() << " messages";
    chan->acknowledge(toAcknowledge);
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

void OtrProxyChannel::Adaptee::trustFingerprint(const QString& fingerprint, bool trust,
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::TrustFingerprintContextPtr &context)
{
    // TODO implement
    Q_UNUSED(fingerprint);
    Q_UNUSED(trust);

    context->setFinished();
}

void OtrProxyChannel::Adaptee::onMessageReceived(const Tp::ReceivedMessage &receivedMessage)
{
    const uint id = receivedMessage.header()[QLatin1String("pending-message-id")].variant().toUInt(nullptr);
    kDebug() << "Received message: " << receivedMessage.text() << " with id: " << id;
    messages.insert(id, receivedMessage);
    Q_EMIT messageReceived(receivedMessage.parts());
}

void OtrProxyChannel::Adaptee::onPendingMessageRemoved(const Tp::ReceivedMessage &receivedMessage)
{
    const uint id = receivedMessage.header().value(QLatin1String("pending-message-id")).variant().toUInt(nullptr);
    if(messages.remove(id)) {
        Q_EMIT pendingMessagesRemoved(Tp::UIntList() << id);
    } else {
        kDebug() << "Text channel removed missing pending message with id: " << id;
    }
}

void OtrProxyChannel::Adaptee::onPendingSendFinished(Tp::PendingOperation *op)
{
    PendingSendMessageResult *sendResult = dynamic_cast<PendingSendMessageResult*>(op);

    Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr ctx = sendResult->getContext();

    if(sendResult->isError()) {
        ctx->setFinishedWithError(sendResult->errorName(), sendResult->errorMessage());
    } else {
        ctx->setFinished(sendResult->getToken());
        Q_EMIT messageSent(sendResult->getMessage(), sendResult->getFlags(), sendResult->getToken());
    }
}

