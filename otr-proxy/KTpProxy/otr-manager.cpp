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

#include "otr-manager.h"
#include "otr-utils.h"
#include "otr-constants.h"
#include "ktp-proxy-debug.h"

#include "KTp/OTR/constants.h"

namespace OTR
{

namespace {

    /** OTR ops functions ------------------------------------------------------------------------- */
    OtrlPolicy policy(void *opdata, ConnContext *context)
    {
        Q_UNUSED(context);

        Session *session = reinterpret_cast<Session*>(opdata);
        return session->parent()->getPolicy();
    }

    void create_privkey(void *opdata, const char *accountname, const char *protocol)
    {
        Q_UNUSED(accountname);
        Q_UNUSED(protocol);
        qCDebug(KTP_PROXY);

        Session *session = reinterpret_cast<Session*>(opdata);
        session->parent()->createNewPrivateKey(session);
    }

    int is_logged_in(void *opdata, const char *accountname, const char *protocol, const char *recipient)
    {
        Q_UNUSED(accountname);
        Q_UNUSED(protocol);
        Q_UNUSED(recipient);

        Session *session = reinterpret_cast<Session*>(opdata);
        return session->recipientStatus();
    }

    void inject_message(void *opdata, const char *accountname,
            const char *protocol, const char *recipient, const char *message)
    {
        Q_UNUSED(accountname);
        Q_UNUSED(protocol);
        Q_UNUSED(recipient);

        Message msg;
        msg.setText(QLatin1String(message));
        msg.setType(Tp::ChannelTextMessageTypeNormal);
        msg.setDirection(MessageDirection::TO_PEER);

        Session *session = reinterpret_cast<Session*>(opdata);
        session->handleMessage(msg);
    }

    void update_context_list(void *opdata)
    {
        Q_UNUSED(opdata);
        // FIXME - all changes in state are caught gone_secure/gone_insecure
    }

    void new_fingerprint(void *opdata, OtrlUserState us,
            const char *accountname, const char *protocol,
            const char *username, unsigned char fingerprint[20])
    {
        Q_UNUSED(us);
        Q_UNUSED(accountname);
        Q_UNUSED(protocol);
        Q_UNUSED(username);

        Session *session = reinterpret_cast<Session*>(opdata);
        session->onNewFingerprintReceived(OTR::utils::humanReadable(fingerprint));
    }

    void write_fingerprints(void *opdata)
    {
        Session *session = reinterpret_cast<Session*>(opdata);
        session->parent()->saveFingerprints(session);
    }

    void gone_secure(void *opdata, ConnContext *context)
    {
        Session *session = reinterpret_cast<Session*>(opdata);
        session->onTrustLevelChanged(context);
    }

    void gone_insecure(void *opdata, ConnContext *context)
    {
        Session *session = reinterpret_cast<Session*>(opdata);
        session->onTrustLevelChanged(context);
    }

    void still_secure(void *opdata, ConnContext *context, int is_reply)
    {
        Q_UNUSED(context);
        Q_UNUSED(is_reply);

        Session *session = reinterpret_cast<Session*>(opdata);
        session->onSessionRefreshed();
    }

    int max_message_size(void *opdata, ConnContext *context)
    {
        Q_UNUSED(context);

        Session *session = reinterpret_cast<Session*>(opdata);
        return session->maxMessageSize();
    }

    const char* otr_error_message(void *opdata, ConnContext *context,
            OtrlErrorCode err_code)
    {
        Q_UNUSED(opdata);

        char *err_msg = 0;
        switch (err_code)
        {
            case OTRL_ERRCODE_NONE:
                break;
            case OTRL_ERRCODE_ENCRYPTION_ERROR:
                {
                    QString message = QLatin1String("Error occurred encrypting message.");
                    err_msg = new char[message.length() + 1];
                    err_msg[message.length()] = 0;
                    memcpy(err_msg, message.toUtf8().data(), message.length());
                    break;
                }
            case OTRL_ERRCODE_MSG_NOT_IN_PRIVATE:
                if (context) {
                    QString message = QString::fromLatin1("You sent encrypted data to %1, who wasn't expecting it.").
                        arg(QLatin1String(context->accountname));
                    err_msg = new char[message.length() + 1];
                    err_msg[message.length()] = 0;
                    memcpy(err_msg, message.toUtf8().data(), message.length());
                }
                break;
            case OTRL_ERRCODE_MSG_UNREADABLE:
                {
                    QString message = QLatin1String("You transmitted an unreadable encrypted message.");
                    err_msg = new char[message.length() + 1];
                    err_msg[message.length()] = 0;
                    memcpy(err_msg, message.toUtf8().data(), message.length());
                    break;
                }
            case OTRL_ERRCODE_MSG_MALFORMED:
                {
                    QString message = QLatin1String("You transmitted a malformed data message.");
                    err_msg = new char[message.length() + 1];
                    err_msg[message.length()] = 0;
                    memcpy(err_msg, message.toUtf8().data(), message.length());
                    break;
                }
        }

        return err_msg;
    }

    void otr_error_message_free(void *opdata, const char *err_msg)
    {
        Q_UNUSED(opdata);

        if(err_msg) {
            delete [] const_cast<char*>(err_msg);
        }
    }

    const char* resent_msg_prefix(void *opdata, ConnContext *context)
    {
        Q_UNUSED(opdata);
        Q_UNUSED(context);

        return "[resent]";
    }

    void resent_msg_prefix_free(void *opdata, const char *prefix)
    {
        Q_UNUSED(opdata);
        Q_UNUSED(prefix);

        return;
    }

    void handle_smp_event(void *opdata, OtrlSMPEvent smp_event,
            ConnContext *context, unsigned short progress_percent,
            char *question)
    {
        Q_UNUSED(progress_percent);

        Session *session = reinterpret_cast<Session*>(opdata);
        qCDebug(KTP_PROXY) << session->context().accountName;

        switch (smp_event) {
            case OTRL_SMPEVENT_NONE:
                break;
            case OTRL_SMPEVENT_ASK_FOR_SECRET:
                session->onSMPQuery(QLatin1String(""));
                break;
            case OTRL_SMPEVENT_ASK_FOR_ANSWER:
                session->onSMPQuery(QLatin1String(question));
                break;
            case OTRL_SMPEVENT_IN_PROGRESS:
                session->onSMPInProgress();
                break;
            case OTRL_SMPEVENT_SUCCESS:
                session->onSMPFinished(true);
                session->onTrustLevelChanged();
                break;
            case OTRL_SMPEVENT_FAILURE:
                session->onSMPFinished(false);
                session->onTrustLevelChanged();
                break;
            case OTRL_SMPEVENT_ABORT:
                session->abortSMPAuthentiaction(context);
                session->onSMPAborted();
                break;
            case OTRL_SMPEVENT_CHEATED:
                session->abortSMPAuthentiaction(context);
                session->onSMPCheated();
                break;
            case OTRL_SMPEVENT_ERROR:
                session->abortSMPAuthentiaction(context);
                session->onSMPError();
                break;
        }
    }

    void handle_msg_event(void *opdata, OtrlMessageEvent msg_event,
            ConnContext *context, const char *message,
            gcry_error_t err)
    {
        Message msg;
        msg.setType(Tp::ChannelTextMessageTypeNotice);
        msg.setOTRevent(msg_event);

        switch (msg_event)
        {
            case OTRL_MSGEVENT_NONE:
                return;
            case OTRL_MSGEVENT_ENCRYPTION_REQUIRED:
                msg.setText(QString::fromLatin1("You attempted to send an unencrypted message to %1.")
                        .arg(QLatin1String(context->username)));
                msg.setDirection(MessageDirection::INTERNAL);
                break;
            case OTRL_MSGEVENT_ENCRYPTION_ERROR:
                msg.setText(QString::fromLatin1("An error occurred when encrypting your message. "
                            "The message was not sent."));
                msg.setDirection(MessageDirection::INTERNAL);
                break;
            case OTRL_MSGEVENT_CONNECTION_ENDED:
                msg.setText(QString::fromLatin1("%1 has already closed his/her private connection to you. "
                            "Your message was not sent. "
                            "Either end your private conversation, or restart it.")
                        .arg(QLatin1String(context->username)));
                msg.setDirection(MessageDirection::INTERNAL);
                break;
            case OTRL_MSGEVENT_SETUP_ERROR:
                if(!err) {
                    err = GPG_ERR_INV_VALUE;
                }
                switch (gcry_err_code(err))
                {
                    case GPG_ERR_INV_VALUE:
                        {
                            msg.setOTRheader(OTR_ERROR_HEADER, QLatin1String("Malformed message received"));
                            msg.setText(QLatin1String("Error setting up private conversation: "
                                        "Malformed message received"));
                            break;
                        }
                    default:
                        {
                            msg.setOTRheader(OTR_ERROR_HEADER, QLatin1String(gcry_strerror(err)));
                            msg.setText(QString::fromLatin1("Error setting up private conversation: %1")
                                    .arg(QLatin1String(gcry_strerror(err))));
                            break;
                        }
                }
                msg.setDirection(MessageDirection::INTERNAL);
                break;
            case OTRL_MSGEVENT_MSG_REFLECTED:
                msg.setText(QLatin1String("We are receiving our own OTR messages. "
                            "You are either trying to talk to yourself, "
                            "or someone is reflecting your messages back at you."));
                msg.setDirection(MessageDirection::INTERNAL);
                break;
            case OTRL_MSGEVENT_MSG_RESENT:
                msg.setText(QString::fromLatin1("The last message to %1 was resent.")
                        .arg(QLatin1String(context->username)));
                msg.setDirection(MessageDirection::INTERNAL);
                break;
            case OTRL_MSGEVENT_RCVDMSG_NOT_IN_PRIVATE:
                msg.setText(QString::fromLatin1("The encrypted message received from %1 is unreadable, "
                            "as you are not currently communicating privately.")
                        .arg(QLatin1String(context->username)));
                msg.setDirection(MessageDirection::FROM_PEER);
                break;
            case OTRL_MSGEVENT_RCVDMSG_UNREADABLE:
                msg.setText(QString::fromLatin1("We received an unreadable encrypted message from %1.")
                        .arg(QLatin1String(context->username)));
                msg.setDirection(MessageDirection::INTERNAL);
                break;
            case OTRL_MSGEVENT_RCVDMSG_MALFORMED:
                msg.setText(QString::fromLatin1("We received a malformed data message from %1.")
                        .arg(QLatin1String(context->username)));
                msg.setDirection(MessageDirection::INTERNAL);
                break;
            case OTRL_MSGEVENT_LOG_HEARTBEAT_RCVD:
                return;
            case OTRL_MSGEVENT_LOG_HEARTBEAT_SENT:
                break;
            case OTRL_MSGEVENT_RCVDMSG_GENERAL_ERR:
                msg.setOTRheader(OTR_ERROR_HEADER, QLatin1String(message));
                msg.setText(QLatin1String(message));
                msg.setDirection(MessageDirection::INTERNAL);
                break;
            case OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED:
                msg.setOTRheader(OTR_UNENCRYPTED_MESSAGE_HEADER, QLatin1String(message));
                msg.setText(QString::fromLatin1("The following message received from %1 was not encrypted: [%2]")
                        .arg(QLatin1String(context->username), QLatin1String(message)));
                msg.setDirection(MessageDirection::FROM_PEER);
                break;
            case OTRL_MSGEVENT_RCVDMSG_UNRECOGNIZED:
                break;
            case OTRL_MSGEVENT_RCVDMSG_FOR_OTHER_INSTANCE:
                msg.setText(QString::fromLatin1("%1 has sent an encrypted message intended for a different session. "
                            "If you are logged in multiple times, another session may have received the message.")
                        .arg(QLatin1String(context->username)));
                msg.setDirection(MessageDirection::FROM_PEER);
                break;
        }

        Session *session = reinterpret_cast<Session*>(opdata);
        session->handleMessage(msg);
    }

    void create_instag(void *opdata, const char *accountname,
            const char *protocol)
    {
        Q_UNUSED(accountname);
        Q_UNUSED(protocol);

        Session *session = reinterpret_cast<Session*>(opdata);
        session->parent()->createInstag(session);
    }

    void timer_control(void *opdata, unsigned int interval)
    {
        Session *session = reinterpret_cast<Session*>(opdata);
        session->userStateBox()->setInterval(interval);
    }

} /* anonymous namespace */

/** OTR ops struct ---------------------------------------------------------------------------- */
namespace global
{
    const OtrlMessageAppOps appOps = {
        policy,
        create_privkey,
        is_logged_in,
        inject_message,
        update_context_list,
        new_fingerprint,
        write_fingerprints,
        gone_secure,
        gone_insecure,
        still_secure,
        max_message_size,
        NULL,           /* account_name */
        NULL,           /* account_name_free */
        NULL,           /* received symkey */
        otr_error_message,
        otr_error_message_free,
        resent_msg_prefix,
        resent_msg_prefix_free,
        handle_smp_event,
        handle_msg_event,
        create_instag,
        NULL,           /* convert_data */
        NULL,           /* convert_data_free */
        timer_control
    };
} /* global namespace */

/** Manager implementation -------------------------------------------------------------------- */
Manager::Manager(Config *otrConfig)
    : config(otrConfig)
{
}

UserStateBox* Manager::getUserState(const QString &accountId)
{
    auto usIt = userStates.find(accountId);
    if(usIt == userStates.end()) {
        // initiate new userstate
        OtrlUserState userstate = otrl_userstate_create();

        QString path = config->saveLocation() + accountId + QLatin1String(".privkeys");
        otrl_privkey_read(userstate, path.toLocal8Bit());

        path = config->saveLocation() + accountId + QLatin1String(".fingerprints");
        otrl_privkey_read_fingerprints(userstate, path.toLocal8Bit(), NULL, NULL);

        path = config->saveLocation() + accountId + QLatin1String(".instags");
        otrl_instag_read(userstate, path.toLocal8Bit());

        UserStateBoxPtr usPtr(new UserStateBox(userstate));
        userStates.insert(accountId, usPtr);
        return usPtr.data();
    } else {
        return usIt->data();
    }
}

OtrlPolicy Manager::getPolicy() const
{
    return config->getPolicy();
}

void Manager::setPolicy(OtrlPolicy policy)
{
    config->setPolicy(policy);
}

void Manager::createNewPrivateKey(Session *session)
{
    const QString path = config->saveLocation() + session->context().accountId + QLatin1String(".privkeys");
    otrl_privkey_generate(session->userStateBox()->userState(),
            path.toLocal8Bit(),
            session->context().accountName.toLocal8Bit(),
            session->context().protocol.toLocal8Bit());
}

KeyGenerationWorker* Manager::createNewPrivateKey(const QString &accountId, const QString &accountName)
{
    const QString path = config->saveLocation() + accountId + QLatin1String(".privkeys");
    return new KeyGenerationWorker(
            accountId,
            accountName,
            utils::protocolFromAccountId(accountId),
            path,
            getUserState(accountId)->userState());
}

QString Manager::getFingerprintFor(const QString &accountId, const QString &accountName)
{
    OtrlUserState userState = getUserState(accountId)->userState();
    unsigned char ourRawHash[20];
    unsigned char *res = otrl_privkey_fingerprint_raw(
            userState,
            ourRawHash,
            accountName.toLocal8Bit(),
            OTR::utils::protocolFromAccountId(accountId).toLocal8Bit());

    if(res == nullptr) {
        return QLatin1String("");
    } else {
        return utils::humanReadable(ourRawHash);
    }
}

void Manager::saveFingerprints(const QString &accountId)
{
    OtrlUserState userState = getUserState(accountId)->userState();
    const QString path = config->saveLocation() + accountId + QLatin1String(".fingerprints");
	otrl_privkey_write_fingerprints(userState, path.toLocal8Bit());
}

void Manager::saveFingerprints(Session *session)
{
    const QString path = config->saveLocation() + session->context().accountId + QLatin1String(".fingerprints");
	otrl_privkey_write_fingerprints(session->userStateBox()->userState(), path.toLocal8Bit());
}

void Manager::createInstag(Session *session)
{
    const QString path = config->saveLocation() + session->context().accountId + QLatin1String(".instags");
	otrl_instag_generate(session->userStateBox()->userState(),
            path.toLocal8Bit(),
            session->context().accountName.toLocal8Bit(),
            session->context().protocol.toLocal8Bit());
}

TrustFpResult Manager::trustFingerprint(const SessionContext &ctx, Fingerprint *fingerprint, bool trust)
{
    if(fingerprint == nullptr) {
        return TrustFpResult::NO_SUCH_FINGERPRINT;
    }

    UserStateBox* usBox = getUserState(ctx.accountId);
    if(trust) {
        otrl_context_set_trust(fingerprint, "VERIFIED");
    } else {
        otrl_context_set_trust(fingerprint, NULL);
    }

    const QString path = config->saveLocation() + ctx.accountId + QLatin1String(".fingerprints");
	otrl_privkey_write_fingerprints(usBox->userState(), path.toLocal8Bit());

    return TrustFpResult::OK;
}

KTp::FingerprintInfoList Manager::getKnownFingerprints(const QString &accountId)
{
    KTp::FingerprintInfoList fingerprints;

    for(ConnContext *context = getUserState(accountId)->userState()->context_root;
            context != nullptr; context = context->next)
    {
        for(Fingerprint *fingerprint = context->fingerprint_root.next;
                fingerprint != nullptr; fingerprint = fingerprint->next)
        {
            const QString username = QLatin1String(context->username);
            const QString hrFp = utils::humanReadable(fingerprint->fingerprint);
            const bool trusted = otrl_context_is_fingerprint_trusted(fingerprint);
            const bool used = utils::isFingerprintInUse(fingerprint);

            fingerprints << KTp::FingerprintInfo { username, hrFp, trusted, used };
        }
    }

    return fingerprints;
}

bool Manager::trustFingerprint(const QString &accountId, const QString &contactName, const QString &fp, bool trust)
{
    Fingerprint *fingerprint = utils::findFingerprint(
            getUserState(accountId)->userState(), fp, contactName);

    if(fingerprint != nullptr) {
        if(trust) {
            otrl_context_set_trust(fingerprint, "VERIFIED");
        } else {
            otrl_context_set_trust(fingerprint, NULL);
        }

        Q_EMIT fingerprintTrusted(accountId, fp, trust);
        return true;
    }

    return false;
}

bool Manager::forgetFingerprint(const QString &accountId, const QString &contactName, const QString &fingerprint)
{
    Fingerprint *fp = utils::findFingerprint(
            getUserState(accountId)->userState(), fingerprint, contactName);

    if(fp != nullptr && !utils::isFingerprintInUse(fp)) {
        otrl_context_forget_fingerprint(fp, 1);
        saveFingerprints(accountId);

        return true;
    }

    return false;
}

KeyGenerationWorker::KeyGenerationWorker(
        const QString &accountId,
        const QString &accountName,
        const QString &protocol,
        const QString &path,
        OtrlUserState userState)
    : accountId(accountId),
    accountName(accountName),
    protocol(protocol),
    path(path),
    userState(userState),
    err(0)
{
}

gcry_error_t KeyGenerationWorker::prepareCreation()
{
    return err = otrl_privkey_generate_start(userState, accountName.toLocal8Bit(), protocol.toLocal8Bit(), &newKey);
}

void KeyGenerationWorker::calculate()
{
    if(!err) {
        err = otrl_privkey_generate_calculate(newKey);
    }
    Q_EMIT finished();
}

gcry_error_t KeyGenerationWorker::error() const
{
    return err;
}

gcry_error_t KeyGenerationWorker::finalizeCreation()
{
    if(!err) {
        return err = otrl_privkey_generate_finish(userState, newKey, path.toLocal8Bit());
    } else {
        return err;
    }
}


} /* namespace OTR */
