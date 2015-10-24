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

#ifndef KTP_PROXY_OTR_SESSION_HEADER
#define KTP_PROXY_OTR_SESSION_HEADER

#include "otr-message.h"
#include "otr-constants.h"
#include "otr-proxy-channel.h"
#include "otr-utils.h"

#include <QString>
#include <QTimer>

extern "C" {
#include <gcrypt.h>
#include <libotr/userstate.h>
}

namespace OTR
{
    class Manager;

    class UserStateBox : public QObject
    {
        Q_OBJECT
        public:
            UserStateBox(OtrlUserState userState);
            ~UserStateBox();

            OtrlUserState userState();
            /** if zero timer is stopped */
            void setInterval(uint interval);

        private Q_SLOTS:
            void otrlMessagePoll();

        private:
            OtrlUserState us;
            QTimer periodicTimer;
    };
    typedef QSharedPointer<UserStateBox> UserStateBoxPtr;

    class Session : public QObject
    {
        Q_OBJECT

        public:
            Session(const SessionContext &context, Manager *parent);
            virtual ~Session() = default;

            UserStateBox* userStateBox() const;
            Manager* parent() const;
            TrustLevel trustLevel() const;
            const SessionContext& context() const;
            QString remoteFingerprint() const;
            QString localFingerprint() const;
            /** forces OTR session into plain text state */
            void forceUnencrypted();

            /** Returns OTR init message */
            Message startSession();
            void stopSession();
            CryptResult encrypt(Message &message);
            CryptResult decrypt(Message &message);

            void initSMPQuery(const QString &question, const QString &secret);
            void initSMPSecret(const QString &secret);
            void abortSMPAuthentiaction(ConnContext *context = NULL);
            void respondSMPAuthentication(const QString &answer);

            TrustFpResult trustFingerprint(bool trust);

            // functions called by libotr ---------------------------------------
            virtual void handleMessage(const Message &message) = 0;

            /** Report whether you think the given user is online.  Return 1 if
              you think he is, 0 if you think he isn't, -1 if you're not sure. */
            virtual int recipientStatus() const = 0;
            virtual unsigned int maxMessageSize() const = 0;

            void onTrustLevelChanged(const ConnContext *context = nullptr);
            void onSessionRefreshed();
            void onNewFingerprintReceived(const QString &fingeprint);

            void onSMPFinished(bool success);
            void onSMPInProgress();
            void onSMPError();
            void onSMPAborted();
            void onSMPCheated();
            void onSMPQuery(const QString &question);

        private:
            Fingerprint* getFingerprint() const;
            ConnContext* findContext() const;

        private Q_SLOTS:
            void onFingerprintTrusted(const QString &accountId, const QString &fingerprint, bool trusted);

        Q_SIGNALS:
            void trustLevelChanged(TrustLevel trustLevel);
            void sessionRefreshed();
            void newFingerprintReceived(const QString &fingeprint);

            void authenticationConcluded(bool success);
            void authenticationError();
            void authenticationAborted();
            void authenticationCheated();
            /** empty string if secret */
            void authenticationRequested(const QString &question);
            void authenticationInProgress();

        private:
            otrl_instag_t instance;
            SessionContext ctx;
            UserStateBox *userstate;
            Manager *pr;
    };
    typedef QSharedPointer<Session> SessionPtr;

    class ProxySession : public Session
    {
        public:
            ProxySession(OtrProxyChannel::Adaptee *pca, const SessionContext &ctx, Manager *parent);

            virtual void handleMessage(const Message &message) override;
            virtual int recipientStatus() const override;
            virtual unsigned int maxMessageSize() const override;

        private:
            OtrProxyChannel::Adaptee *pca;
    };

} /* namespace OTR */

#endif
