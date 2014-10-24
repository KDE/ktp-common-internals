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

#ifndef KTP_PROXY_OTR_PROXY_CHANNEL_ADAPTEE_HEADER
#define KTP_PROXY_OTR_PROXY_CHANNEL_ADAPTEE_HEADER

#include "svc-channel-proxy.h"
#include "otr-proxy-channel.h"
#include "otr-session.h"
#include "otr-message.h"

#include <TelepathyQt/ReceivedMessage>

#include <QDBusObjectPath>
#include <QDBusConnection>

using namespace OTR;

class ProxyService;

class OtrProxyChannel::Adaptee : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDBusObjectPath wrappedChannel READ wrappedChannel)
    Q_PROPERTY(bool connected READ connected)
    Q_PROPERTY(Tp::MessagePartListList pendingMessages READ pendingMessages)
    Q_PROPERTY(uint trustLevel READ trustLevel)
    Q_PROPERTY(QString localFingerprint READ localFingerprint)
    Q_PROPERTY(QString remoteFingerprint READ remoteFingerprint)

    public:
        Adaptee(OtrProxyChannel *pc,
                const QDBusConnection &dbusConnection,
                const Tp::TextChannelPtr &channel,
                const OTR::SessionContext &context,
                ProxyService *ps);

        QDBusObjectPath wrappedChannel() const;
        bool connected() const;
        Tp::MessagePartListList pendingMessages() const;
        uint trustLevel() const;
        QString localFingerprint() const;
        QString remoteFingerprint() const;

        Tp::TextChannelPtr channel() const;

        void processOTRmessage(const OTR::Message &message);
    private:
        void sendOTRmessage(const OTR::Message &message);
        void acquirePrivateKey();

    private Q_SLOTS:
        void connectProxy(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::ConnectProxyContextPtr &context);
        void disconnectProxy(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::DisconnectProxyContextPtr &context);
        void sendMessage(const Tp::MessagePartList &message, uint flags,
                const Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr &context);
        void acknowledgePendingMessages(const Tp::UIntList &ids,
                const Tp::Service::ChannelProxyInterfaceOTRAdaptor::AcknowledgePendingMessagesContextPtr &context);

        void initialize(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::InitializeContextPtr &context);
        void stop(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::StopContextPtr &context);
        void trustFingerprint(const QString& fingerprint, bool trust,
                const Tp::Service::ChannelProxyInterfaceOTRAdaptor::TrustFingerprintContextPtr &context);

        void startPeerAuthentication(const QString &question, const QString &secret,
                const Tp::Service::ChannelProxyInterfaceOTRAdaptor::StartPeerAuthenticationContextPtr &ctx);
        void respondPeerAuthentication(const QString &secret,
                const Tp::Service::ChannelProxyInterfaceOTRAdaptor::RespondPeerAuthenticationContextPtr &ctx);
        void abortPeerAuthentication(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::AbortPeerAuthenticationContextPtr &ctx);

        void onMessageReceived(const Tp::ReceivedMessage &receivedMessage);
        void onPendingMessageRemoved(const Tp::ReceivedMessage &receivedMessage);
        void onPendingSendFinished(Tp::PendingOperation *pendingSend);

        void onTrustLevelChanged(TrustLevel trustLevel);
        void onKeyGenerationStarted(const QString &accountId);
        void onKeyGenerationFinished(const QString &accountId, bool error);

        void onChannelClosed();

    Q_SIGNALS:
        void messageSent(const Tp::MessagePartList &content, uint flags, const QString &messageToken);
        void messageReceived(const Tp::MessagePartList &message);
        void pendingMessagesRemoved(const Tp::UIntList &ids);
        void sessionRefreshed();
        void closed();
        void trustLevelChanged(uint trustLevel);
        void peerAuthenticationRequested(const QString&);
        void peerAuthenticationInProgress();
        void peerAuthenticationConcluded(bool);
        void peerAuthenticationAborted();
        void peerAuthenticationError();
        void peerAuthenticationCheated();

    private:
        Tp::Service::ChannelProxyInterfaceOTRAdaptor *adaptor;
        OtrProxyChannel *pc;
        Tp::TextChannelPtr chan;
        bool isConnected;
        OTR::ProxySession otrSes;
        ProxyService *ps;
        uint sender;

        QMap<uint, Tp::ReceivedMessage> messages; // queue
        QList<Tp::ReceivedMessage> enqueuedMessages; // when there is now private key generated
        bool isGenerating; // is the new private key for this account being generated
        bool aboutToInit; // user wanted to start the OTR session but no private key was available
};

#endif
