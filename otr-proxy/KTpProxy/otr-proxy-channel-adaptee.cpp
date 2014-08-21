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
#include "proxy-service.h"
#include "otr-constants.h"
#include "otr-manager.h"
#include "otr-utils.h"
#include "pending-curry-operation.h"

#include "KTp/OTR/constants.h"

#include <TelepathyQt/DBusObject>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Contact>

#include <QDateTime>

#include <KDebug>


class PendingSendMessageResult : public PendingCurryOperation
{
    public:
        PendingSendMessageResult(Tp::PendingOperation *op,
                const Tp::TextChannelPtr &chan,
                const Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr &context,
                const Tp::MessagePartList &message,
                uint flags,
                bool isOTRmessage = false)
            : PendingCurryOperation(op, Tp::SharedPtr<Tp::RefCounted>::dynamicCast(chan)),
            token(QString::fromLatin1("")),
            context(context),
            message(message),
            flags(flags),
            isOTRmessage(isOTRmessage)
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
    public:
        const bool isOTRmessage;
};


OtrProxyChannel::Adaptee::Adaptee(OtrProxyChannel *pc,
        const QDBusConnection &dbusConnection,
        const Tp::TextChannelPtr &channel,
        const OTR::SessionContext &context,
        ProxyService *ps)
    : QObject(pc),
    adaptor(new Tp::Service::ChannelProxyInterfaceOTRAdaptor(dbusConnection, this, pc->dbusObject())),
    pc(pc),
    chan(channel),
    isConnected(false),
    otrSes(this, context, ps->managerOTR()),
    ps(ps),
    isGenerating(false),
    aboutToInit(false)
{
    kDebug() << "Created OTR session for context: "
        << "Account id: " << context.accountId
        << " Account name: " << context.accountName
        << " recipient name: " << context.recipientName
        << " protocol: " << context.protocol;
    connect(chan.data(), SIGNAL(invalidated(Tp::DBusProxy*,const QString&,const QString&)), SLOT(onChannelClosed()));
    connect(&otrSes, SIGNAL(trustLevelChanged(TrustLevel)), SLOT(onTrustLevelChanged(TrustLevel)));
    connect(&otrSes, SIGNAL(sessionRefreshed()), SIGNAL(sessionRefreshed()));
    connect(&otrSes, SIGNAL(authenticationRequested(const QString&)), SIGNAL(peerAuthenticationRequested(const QString&)));
    connect(&otrSes, SIGNAL(authenticationInProgress()), SIGNAL(peerAuthenticationInProgress()));
    connect(&otrSes, SIGNAL(authenticationConcluded(bool)), SIGNAL(peerAuthenticationConcluded(bool)));
    connect(&otrSes, SIGNAL(authenticationAborted()), SIGNAL(peerAuthenticationAborted()));
    connect(&otrSes, SIGNAL(authenticationError()), SIGNAL(peerAuthenticationError()));
    connect(&otrSes, SIGNAL(authenticationCheated()), SIGNAL(peerAuthenticationCheated()));

    sender = channel->connection()->selfHandle();
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
    return static_cast<uint>(otrSes.trustLevel());
}

QString OtrProxyChannel::Adaptee::localFingerprint() const
{
    return otrSes.localFingerprint();
}

QString OtrProxyChannel::Adaptee::remoteFingerprint() const
{
    return otrSes.remoteFingerprint();
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

    connect(ps, SIGNAL(keyGenerationStarted(const QString&)), SLOT(onKeyGenerationStarted(const QString&)));
    connect(ps, SIGNAL(keyGenerationFinished(const QString&, bool)), SLOT(onKeyGenerationFinished(const QString&, bool)));

    for(const Tp::ReceivedMessage &m: chan->messageQueue()) {
        onMessageReceived(m);
    }

    isConnected = true;
    context->setFinished();
}
void OtrProxyChannel::Adaptee::disconnectProxy(
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::DisconnectProxyContextPtr &context)
{
    if(otrSes.trustLevel() != OTR::TrustLevel::NOT_PRIVATE) {
        otrSes.stopSession();
    }
    kDebug() << "Disconnecting proxy: " << pc->objectPath();
    disconnect(chan.data(), SIGNAL(messageReceived(const Tp::ReceivedMessage&)),
            this, SLOT(onMessageReceived(const Tp::ReceivedMessage&)));

    disconnect(chan.data(), SIGNAL(pendingMessageRemoved(const Tp::ReceivedMessage&)),
            this, SLOT(onPendingMessageRemoved(const Tp::ReceivedMessage&)));

    disconnect(ps, SIGNAL(keyGenerationStarted(const QString&)),
            this, SLOT(onKeyGenerationStarted(const QString&)));
    disconnect(ps, SIGNAL(keyGenerationFinished(const QString&, bool)), this,
            SLOT(onKeyGenerationFinished(const QString&, bool)));

    isConnected = false;
    messages.clear();
    context->setFinished();
}

void OtrProxyChannel::Adaptee::processOTRmessage(const OTR::Message &message)
{
    kDebug();
    switch(message.direction()) {
        case OTR::MessageDirection::INTERNAL:
        case OTR::MessageDirection::FROM_PEER:
            Q_EMIT messageReceived(message.parts());
            return;
        case OTR::MessageDirection::TO_PEER:
            sendOTRmessage(message);
            return;
    }
}

void OtrProxyChannel::Adaptee::sendOTRmessage(const OTR::Message &message)
{
    kDebug();
    uint flags = 0;
    PendingSendMessageResult *pending = new PendingSendMessageResult(
            chan->send(message.parts(), (Tp::MessageSendingFlags) flags),
            chan,
            Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr(),
            message.parts(),
            flags,
            true);

    connect(pending,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onPendingSendFinished(Tp::PendingOperation*)));
}

void OtrProxyChannel::Adaptee::sendMessage(const Tp::MessagePartList &message, uint flags,
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr &context)
{
    kDebug();

    if(!connected()) {
        context->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }

    OTR::Message otrMessage(message);
    const OTR::CryptResult cres = otrSes.encrypt(otrMessage);
    if(cres == OTR::CryptResult::ERROR) {
        kDebug() << "Sending error";
        context->setFinishedWithError(KTP_PROXY_ERROR_ENCRYPTION_ERROR,
                QLatin1String("Message could not be encrypted with OTR"));
        return;
    }

    // we are starting an AKE - do not show it to the user
    // policy is probably set to ALWAYS in this case
    if(otrl_proto_message_type(otrMessage.text().toLocal8Bit()) == OTRL_MSGTYPE_QUERY) {
        sendOTRmessage(otrMessage);
    } else {
        PendingSendMessageResult *pending = new PendingSendMessageResult(
                chan->send(otrMessage.parts(), (Tp::MessageSendingFlags) flags),
                chan,
                context,
                message,
                flags);

        connect(pending,
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onPendingSendFinished(Tp::PendingOperation*)));
    }
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
    kDebug();
    if(!connected()) {
        context->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }

    // create private key if necessary - to avoid blocking
    if(otrSes.localFingerprint().isEmpty() && ps->getPolicy() != OTRL_POLICY_NEVER) {
        aboutToInit = true;
        acquirePrivateKey();
    } else if(isGenerating) {
        aboutToInit = true;
    } else {
        sendOTRmessage(otrSes.startSession());
    }

    context->setFinished();
}

void OtrProxyChannel::Adaptee::stop(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::StopContextPtr &context)
{
    if(!connected()) {
        context->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }
    otrSes.stopSession();
    context->setFinished();
}

void OtrProxyChannel::Adaptee::trustFingerprint(const QString& fingerprint, bool trust,
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::TrustFingerprintContextPtr &context)
{
    if(!connected()) {
        context->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }

    kDebug() << "TrustFingeprint - " << trust << ": " << fingerprint << " when remote is: " << otrSes.remoteFingerprint();

    if(otrSes.remoteFingerprint().isEmpty() || fingerprint != otrSes.remoteFingerprint()) {
        context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT,
                QLatin1String("No such fingerprint currently in use by remote contact"));
        return;
    }

    OTR::TrustFpResult fpRes = otrSes.trustFingerprint(trust);
    if(fpRes != OTR::TrustFpResult::OK) {
        // should not happend, TODO clarify
        kDebug() << "Trust error";
        context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT,
                QLatin1String("No such fingerprint currently in use by remote contact"));
        return;
    }
    context->setFinished();
}

void OtrProxyChannel::Adaptee::startPeerAuthentication(const QString &question, const QString &secret,
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::StartPeerAuthenticationContextPtr &ctx)
{
    if(!connected()) {
        ctx->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }

    if(question.isEmpty()) {
        otrSes.initSMPSecret(secret);
    } else {
        otrSes.initSMPQuery(question, secret);
    }
    ctx->setFinished();
}

void OtrProxyChannel::Adaptee::respondPeerAuthentication(const QString &secret,
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::RespondPeerAuthenticationContextPtr &ctx)
{
    if(!connected()) {
        ctx->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }

    otrSes.respondSMPAuthentication(secret);
    ctx->setFinished();
}

void OtrProxyChannel::Adaptee::abortPeerAuthentication(
        const Tp::Service::ChannelProxyInterfaceOTRAdaptor::AbortPeerAuthenticationContextPtr &ctx)
{
    if(!connected()) {
        ctx->setFinishedWithError(KTP_PROXY_ERROR_NOT_CONNECTED, QString::fromLatin1("Proxy is not connected"));
        return;
    }

    otrSes.abortSMPAuthentiaction();
    ctx->setFinished();
}

void OtrProxyChannel::Adaptee::onMessageReceived(const Tp::ReceivedMessage &receivedMessage)
{
    const uint id = receivedMessage.header()[QLatin1String("pending-message-id")].variant().toUInt(nullptr);
    OTR::Message otrMsg(receivedMessage.parts());
    // no private key - should generate on now
    if(otrMsg.isOTRmessage() && otrSes.localFingerprint().isEmpty() && ps->getPolicy() != OTRL_POLICY_NEVER) {
        enqueuedMessages << receivedMessage;
        acquirePrivateKey();
        return;
    // private key is currently being generated
    } else if(isGenerating) {
        enqueuedMessages << receivedMessage;
        return;
    }

    kDebug() << "Received message with id: " << id;
    const OTR::CryptResult cres = otrSes.decrypt(otrMsg);

    if(cres == OTR::CryptResult::CHANGED || cres == OTR::CryptResult::UNCHANGED) {
        messages.insert(id, receivedMessage);

        Q_EMIT messageReceived(otrMsg.parts());
    } else {
        // Error or OTR message - acknowledge right now
        if(cres == OTR::CryptResult::ERROR) {
            kWarning() << "Decryption error of the message: " << otrMsg.text();
        }
        chan->acknowledge(QList<Tp::ReceivedMessage>() << receivedMessage);
    }
}

void OtrProxyChannel::Adaptee::onPendingMessageRemoved(const Tp::ReceivedMessage &receivedMessage)
{
    const uint id = receivedMessage.header().value(QLatin1String("pending-message-id")).variant().toUInt(nullptr);
    if(messages.remove(id)) {
        Q_EMIT pendingMessagesRemoved(Tp::UIntList() << id);
    } else {
        kDebug() << "Text channel removed missing pending message with id or an OTR message: " << id;
    }
}

void OtrProxyChannel::Adaptee::onPendingSendFinished(Tp::PendingOperation *op)
{
    PendingSendMessageResult *sendResult = dynamic_cast<PendingSendMessageResult*>(op);

    if(sendResult->isOTRmessage) {
        if(sendResult->isError()) {
            // TODO check the message and provide information to the user
        }
    } else {
        Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr ctx = sendResult->getContext();

        if(sendResult->isError()) {
            ctx->setFinishedWithError(sendResult->errorName(), sendResult->errorMessage());
        } else {
            ctx->setFinished(sendResult->getToken());

            OTR::Message message = sendResult->getMessage();
            message.setToken(sendResult->getToken());
            message.setTimestamp(QDateTime::currentDateTime().toTime_t());
            message.setSender(sender);
            message.setSenderId(otrSes.context().accountName);
            if(!otrSes.remoteFingerprint().isEmpty()) {
                message.setOTRheader(OTR_REMOTE_FINGERPRINT_HEADER, otrSes.remoteFingerprint());
            }

            Q_EMIT messageSent(message.parts(), sendResult->getFlags(), sendResult->getToken());
        }
    }
}

void OtrProxyChannel::Adaptee::onTrustLevelChanged(TrustLevel trustLevel)
{
    Q_EMIT trustLevelChanged(static_cast<uint>(trustLevel));
}

void OtrProxyChannel::Adaptee::acquirePrivateKey()
{
    if(!ps->createNewPrivateKey(otrSes.context().accountId, otrSes.context().accountName)) {
        kDebug() << "Probably ongoing key generation for another session";
    }
}

void OtrProxyChannel::Adaptee::onKeyGenerationStarted(const QString &accountId)
{
    if(accountId != otrSes.context().accountId) {
        return;
    }
    isGenerating = true;
}

void OtrProxyChannel::Adaptee::onKeyGenerationFinished(const QString &accountId, bool error)
{
    if(accountId != otrSes.context().accountId) {
        return;
    }
    kDebug() << "Finished key generation for: " << accountId;
    isGenerating = false;

    if(error) {
        kWarning() << "Could not generate private key for " << accountId;
        return;
    }
    if(!enqueuedMessages.isEmpty()) {
        for(auto &&mes: enqueuedMessages) {
            onMessageReceived(mes);
        }
        enqueuedMessages.clear();
    } else if(aboutToInit) {
        sendOTRmessage(otrSes.startSession());
    }

    aboutToInit = false;
}

void OtrProxyChannel::Adaptee::onChannelClosed()
{
    kDebug();
    // we will not be able to send disconnect message so we just finish our own OTR session
    if(isConnected) {
        otrSes.forceUnencrypted();
    }
    Q_EMIT closed();
}
