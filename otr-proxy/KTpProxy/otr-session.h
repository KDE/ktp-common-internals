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

            /** Returns OTR init message */
            Message startSession();
            void stopSession();
            CryptResult encrypt(Message &message);
            CryptResult decrypt(Message &message);
            void initSMPQuery();
            void initSMPSecret();
            TrustFpResult trustFingerprint(bool trust);

            // functions called by libotr
            virtual void handleMessage(const Message &message) = 0;
            virtual void handleSmpEvent(OtrlSMPEvent smpEvent) = 0;
            virtual int recipientStatus() const = 0;
            virtual unsigned int maxMessageSize() const = 0;
            void onTrustLevelChanged(TrustLevel trustLevel, const ConnContext *context);
            void onSessionRefreshed();
            void onNewFingerprintReceived(const QString &fingeprint);

        private:
            Fingerprint* getFingerprint() const;

        Q_SIGNALS:
            void trustLevelChanged(TrustLevel trustLevel);
            void sessionRefreshed();
            void newFingerprintReceived(const QString &fingeprint);

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
            virtual void handleSmpEvent(OtrlSMPEvent smpEvent) override;
            virtual int recipientStatus() const override;
            virtual unsigned int maxMessageSize() const override;

        private:
            OtrProxyChannel::Adaptee *pca;
    };

} /* namespace OTR */

#endif
