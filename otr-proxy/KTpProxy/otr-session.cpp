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
#include "otr-utils.h"

extern "C" {
#include <libotr/privkey.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
}

namespace OTR
{
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


    Session::Session(const SessionContext &ctx, Manager *parent)
        : instance(OTRL_INSTAG_BEST),
        ctx(ctx),
        tlevel(TrustLevel::NOT_PRIVATE),
        pr(parent)
    {
        userstate = pr->getUserState(ctx);
    }

    TrustLevel Session::trustLevel() const
    {
        return tlevel;
    }

    UserStateBox* Session::userStateBox()
    {
        return userstate;
    }

    Manager* Session::parent()
    {
        return pr;
    }

    const SessionContext& Session::context() const
    {
        return ctx;
    }

    Fingerprint* Session::getFingerprint() const
    {
        ConnContext *context = otrl_context_find(userstate->userState(),
                ctx.recipientName.toLocal8Bit(),
                ctx.accountName.toLocal8Bit(),
                ctx.protocol.toLocal8Bit(),
                instance, 0, NULL, NULL, NULL);

        if(context && context->active_fingerprint) {
            return context->active_fingerprint;
        } else {
            return nullptr;
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

        onTrustLevelChanged(TrustLevel::NOT_PRIVATE, nullptr);
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

                message.setText(QLatin1String(encMessage));
                message.setType(Tp::ChannelTextMessageTypeNormal);
                if(context->active_fingerprint != nullptr) {
                    const QString hrFingerprint = OTR::utils::humanReadable(context->active_fingerprint->fingerprint);
                    message.setOTRheader(QLatin1String("otr-remote-fingerprint"), hrFingerprint);
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
                message.setText(QLatin1String(decMsg));
                if(context->active_fingerprint != nullptr) {
                    const QString hrFingerprint = OTR::utils::humanReadable(context->active_fingerprint->fingerprint);
                    message.setOTRheader(QLatin1String("otr-remote-fingerprint"), hrFingerprint);
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
            onTrustLevelChanged(TrustLevel::FINISHED, nullptr);
        }

        return result;
    }

    TrustFpResult Session::trustFingerprint(bool trust)
    {
        Fingerprint* fp = getFingerprint();
        if(fp != nullptr) {

            TrustFpResult res = pr->trustFingerprint(ctx, fp, trust);
            if(res == TrustFpResult::OK && trustLevel() != TrustLevel::FINISHED) {
                if(trust) {
                    onTrustLevelChanged(TrustLevel::VERIFIED, nullptr);
                } else {
                    onTrustLevelChanged(TrustLevel::UNVERIFIED, nullptr);
                }
            }
            return res;
        } else {

            return TrustFpResult::NO_SUCH_FINGERPRINT;
        }
    }

    void Session::onTrustLevelChanged(TrustLevel trustLevel, const ConnContext *context)
    {
        if(context != nullptr) {
            instance = context->their_instance;
        }
        tlevel = trustLevel;
        emit trustLevelChanged(trustLevel);
    }

    void Session::onSessionRefreshed()
    {
        emit sessionRefreshed();
    }

    void Session::onNewFingerprintReceived(const QString &fingerprint)
    {
        //qDebug() << "Received";
        emit newFingeprintReceived(fingerprint);
    }

} /* namespace OTR */
