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

#include "otr-session.h"
#include "otr-manager.h"
#include "otr-message.h"
#include "otr-utils.h"
#include "otr-proxy-channel-adaptee.h"

#include "KTp/OTR/constants.h"

extern "C" {
#include <libotr/privkey.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
}

namespace OTR
{
    // -------- UserStateBox --------------------------------------------

    UserStateBox::UserStateBox(OtrlUserState userState)
        : us(userState)
    {
        uint interval = otrl_message_poll_get_default_interval(us);
        QObject::connect(&periodicTimer, SIGNAL(timeout()), SLOT(otrlMessagePoll()));
        periodicTimer.start(interval * 1000);
    }

    UserStateBox::~UserStateBox()
    {
        otrl_userstate_free(us);
    }

    OtrlUserState UserStateBox::userState()
    {
        return us;
    }

    void UserStateBox::setInterval(uint interval)
    {
        if(interval) {
            periodicTimer.start(interval * 1000);
        } else {
            periodicTimer.stop();
        }
    }

    void UserStateBox::otrlMessagePoll()
    {
        otrl_message_poll(us, 0, 0);
    }

    // -------- Session -------------------------------------------------

    Session::Session(const SessionContext &ctx, Manager *parent)
        : instance(OTRL_INSTAG_BEST),
        ctx(ctx),
        pr(parent)
    {
        userstate = pr->getUserState(ctx.accountId);
        connect(parent, SIGNAL(fingerprintTrusted(const QString&, const QString&, bool)),
                SLOT(onFingerprintTrusted(const QString&, const QString&, bool)));
    }

    TrustLevel Session::trustLevel() const
    {
        return OTR::utils::getTrustLevel(ctx, userstate->userState(), instance);
    }

    UserStateBox* Session::userStateBox() const
    {
        return userstate;
    }

    Manager* Session::parent() const
    {
        return pr;
    }

    const SessionContext& Session::context() const
    {
        return ctx;
    }

    Fingerprint* Session::getFingerprint() const
    {
        ConnContext *context = findContext();

        if(context && context->active_fingerprint) {
            return context->active_fingerprint;
        } else {
            return nullptr;
        }
    }

    ConnContext* Session::findContext() const
    {
        return otrl_context_find(userstate->userState(),
                ctx.recipientName.toLocal8Bit(),
                ctx.accountName.toLocal8Bit(),
                ctx.protocol.toLocal8Bit(),
                instance, 0, NULL, NULL, NULL);
    }

    void Session::onFingerprintTrusted(const QString &accountId, const QString &fingerprint, bool trusted)
    {
        Q_UNUSED(trusted);
        if(accountId == ctx.accountId && fingerprint == remoteFingerprint()) {
            onTrustLevelChanged();
        }
    }

    QString Session::remoteFingerprint() const
    {
        Fingerprint *fp = getFingerprint();
        if(fp) {
            return utils::humanReadable(fp->fingerprint);
        } else {
            return QLatin1String("");
        }
    }

    QString Session::localFingerprint() const
    {
        return parent()->getFingerprintFor(ctx.accountId, ctx.accountName);
    }

    void Session::forceUnencrypted()
    {
        if(trustLevel() == TrustLevel::NOT_PRIVATE) {
            return;
        }

        ConnContext *context = findContext();
        otrl_context_force_plaintext(context);
    }

    Message Session::startSession()
    {
        Message msg;

        char *message = otrl_proto_default_query_msg(ctx.accountName.toLocal8Bit(), pr->getPolicy());
        msg.setText(QLatin1String(message));
        msg.setType(Tp::ChannelTextMessageTypeNormal);
        msg.setDirection(MessageDirection::TO_PEER);
        otrl_message_free(message);

        return msg;
    }

    void Session::stopSession()
    {
        otrl_message_disconnect(
                userstate->userState(),
                &global::appOps,
                this,
                ctx.accountName.toLocal8Bit(),
                ctx.protocol.toLocal8Bit(),
                ctx.recipientName.toLocal8Bit(),
                instance);

        onTrustLevelChanged();
    }

    CryptResult Session::encrypt(Message &message)
    {
        if(otrl_proto_message_type(message.text().toLocal8Bit()) == OTRL_MSGTYPE_NOTOTR) {

            char *encMessage = 0;
            ConnContext *context;

            int err = otrl_message_sending(
                    userstate->userState(),
                    &global::appOps,
                    this,
                    ctx.accountName.toLocal8Bit(),
                    ctx.protocol.toLocal8Bit(),
                    ctx.recipientName.toLocal8Bit(),
                    instance,
                    message.text().toLocal8Bit(),
                    NULL,
                    &encMessage,
                    OTRL_FRAGMENT_SEND_ALL_BUT_LAST,
                    &context,
                    NULL,
                    NULL);

            if(err) {
                return CryptResult::ERROR;
            } else if(encMessage != nullptr) {

                if(message.contentType().isEmpty()) {
                    message.setText(QString::fromLocal8Bit(encMessage));
                } else {
                    message.setText(QString::fromLocal8Bit(encMessage), message.contentType());
                }
                message.setType(Tp::ChannelTextMessageTypeNormal);
                if(context->active_fingerprint != nullptr) {
                    const QString hrFingerprint = OTR::utils::humanReadable(context->active_fingerprint->fingerprint);
                    message.setOTRheader(OTR_REMOTE_FINGERPRINT_HEADER, hrFingerprint);
                }
                otrl_message_free(encMessage);

                return CryptResult::CHANGED;
            }
        }

        return CryptResult::UNCHANGED;
    }

    CryptResult Session::decrypt(Message &message)
    {
        CryptResult result = CryptResult::OTR;
        char *decMsg = nullptr;
        OtrlTLV *tlvs = nullptr;
        ConnContext *context = nullptr;

        bool isFinished = false;

        int ignore = otrl_message_receiving(
                userstate->userState(),
                &global::appOps,
                this,
                ctx.accountName.toLocal8Bit(),
                ctx.protocol.toLocal8Bit(),
                ctx.recipientName.toLocal8Bit(),
                message.text().toLocal8Bit(),
                &decMsg,
                &tlvs,
                &context, NULL, NULL);

		if(otrl_tlv_find(tlvs, OTRL_TLV_DISCONNECTED) != nullptr) {
            isFinished = true;
        }
        otrl_tlv_free(tlvs);

        if(!ignore) {
            if(decMsg != nullptr) {
                if(message.contentType().isEmpty()) {
                    message.setText(QString::fromLocal8Bit(decMsg));
                } else {
                    message.setText(QString::fromLocal8Bit(decMsg), message.contentType());
                }
                if(context->active_fingerprint != nullptr) {
                    const QString hrFingerprint = OTR::utils::humanReadable(context->active_fingerprint->fingerprint);
                    message.setOTRheader(OTR_REMOTE_FINGERPRINT_HEADER, hrFingerprint);
                }
                result = CryptResult::CHANGED;
            } else {
                result = CryptResult::UNCHANGED;
            }
        } else {
            result = CryptResult::OTR;
        }

        if(decMsg != nullptr) {
            otrl_message_free(decMsg);
        }

        if(isFinished) {
            onTrustLevelChanged();
        }

        return result;
    }

    void Session::initSMPQuery(const QString &question, const QString &secret)
    {
        otrl_message_initiate_smp_q(userstate->userState(),
                &global::appOps,
                this,
                findContext(),
                (const char*)question.toLocal8Bit().data(),
                (unsigned char*)secret.toLocal8Bit().data(),
                secret.length());
    }

    void Session::initSMPSecret(const QString &secret)
    {
        otrl_message_initiate_smp(userstate->userState(),
                &global::appOps,
                this,
                findContext(),
                (unsigned char*)secret.toLocal8Bit().data(),
                secret.length());
    }

    void Session::abortSMPAuthentiaction(ConnContext *context)
    {
        if(context == NULL) {
            context = findContext();
        }
        otrl_message_abort_smp(userstate->userState(), &global::appOps, this, context);
    }

    void Session::respondSMPAuthentication(const QString &answer)
    {
        otrl_message_respond_smp(userstate->userState(),
                &global::appOps,
                this,
                findContext(),
                (unsigned char*)answer.toLocal8Bit().data(),
                answer.length());
    }

    TrustFpResult Session::trustFingerprint(bool trust)
    {
        Fingerprint* fp = getFingerprint();
        if(fp != nullptr) {

            TrustFpResult res = pr->trustFingerprint(ctx, fp, trust);
            if(res == TrustFpResult::OK && trustLevel() != TrustLevel::FINISHED) {
                onTrustLevelChanged();
            }
            return res;
        } else {

            return TrustFpResult::NO_SUCH_FINGERPRINT;
        }
    }

    void Session::onTrustLevelChanged(const ConnContext *context)
    {
        if(context != nullptr) {
            instance = context->their_instance;
        }
        Q_EMIT trustLevelChanged(trustLevel());
    }

    void Session::onSessionRefreshed()
    {
        Q_EMIT sessionRefreshed();
    }

    void Session::onNewFingerprintReceived(const QString &fingerprint)
    {
        Q_EMIT newFingerprintReceived(fingerprint);
    }

    void Session::onSMPFinished(bool success)
    {
        Q_EMIT authenticationConcluded(success);
    }

    void Session::onSMPInProgress()
    {
        Q_EMIT authenticationInProgress();
    }

    void Session::onSMPError()
    {
        Q_EMIT authenticationError();
    }

    void Session::onSMPAborted()
    {
        Q_EMIT authenticationAborted();
    }

    void Session::onSMPCheated()
    {
        Q_EMIT authenticationCheated();
    }

    void Session::onSMPQuery(const QString &question)
    {
        Q_EMIT authenticationRequested(question);
    }

    // -------- ProxySession --------------------------------------------
    ProxySession::ProxySession(OtrProxyChannel::Adaptee *pca, const SessionContext &ctx, Manager *parent)
        : Session(ctx, parent),
        pca(pca)
    {
    }

    void ProxySession::handleMessage(const Message &message)
    {
        pca->processOTRmessage(message);
    }

    int ProxySession::recipientStatus() const
    {
        if(pca->channel()->hasChatStateInterface()) {
            switch(pca->channel()->chatState(pca->channel()->targetContact())) {
                case Tp::ChannelChatStateGone:
                    return 0;
                default:
                    return 1;
            }
        } else {
            return -1;
        }
    }

    unsigned int ProxySession::maxMessageSize() const
    {
        // FIXME cannot determine maximum message size with Telepathy
        return 0;
    }

} /* namespace OTR */
