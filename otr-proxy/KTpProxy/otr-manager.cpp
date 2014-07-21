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
#include "otr-session.h"

extern "C" {
#include <gcrypt.h>
#include <libotr/privkey.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
}

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

    }

    int is_logged_in(void *opdata, const char *accountname, const char *protocol, const char *recipient)
    {
        Q_UNUSED(accountname);
        Q_UNUSED(protocol);
        Q_UNUSED(recipient);

        Session *session = reinterpret_cast<Session*>(opdata);
        return session->handler()->recipientStatus();
    }

    void inject_message(void *opdata, const char *accountname,
            const char *protocol, const char *recipient, const char *message)
    {

    }

    void update_context_list(void *opdata)
    {

    }

    void new_fingerprint(void *opdata, OtrlUserState us,
            const char *accountname, const char *protocol,
            const char *username, unsigned char fingerprint[20])
    {

    }

    void write_fingerprints(void *opdata)
    {

    }

    void gone_secure(void *opdata, ConnContext *context)
    {

    }

    void gone_insecure(void *opdata, ConnContext *context)
    {

    }

    void still_secure(void *opdata, ConnContext *context, int is_reply)
    {

    }

    int max_message_size(void *opdata, ConnContext *context)
    {
        return 0;
    }

    const char* otr_error_message(void *opdata, ConnContext *context,
            OtrlErrorCode err_code)
    {
        return 0;
    }

    void otr_error_message_free(void *opdata, const char *err_msg)
    {

    }

    const char* resent_msg_prefix(void *opdata, ConnContext *context)
    {
        return 0;
    }

    void resent_msg_prefix_free(void *opdata, const char *prefix)
    {

    }

    void handle_smp_event(void *opdata, OtrlSMPEvent smp_event,
            ConnContext *context, unsigned short progress_percent,
            char *question)
    {

    }

    void handle_msg_event(void *opdata, OtrlMessageEvent msg_event,
            ConnContext *context, const char *message,
            gcry_error_t err)
    {

    }

    void create_instag(void *opdata, const char *accountname,
            const char *protocol)
    {

    }

    void timer_control(void *opdata, unsigned int interval)
    {
        Session *session = reinterpret_cast<Session*>(opdata);
        session->userState()->setInterval(interval);
    }

    /** OTR ops struct ---------------------------------------------------------------------------- */
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

} /* anonymous namespace */


/** Manager implementation -------------------------------------------------------------------- */
Manager::Manager(Config *otrConfig)
    : config(config)
{

}

SessionPtr Manager::createSession(const HandlerPtr &handler)
{
    auto usIt = userStates.find(handler->context().accountId);
    if(usIt == userStates.end()) {
        // initiate new userstate
        OtrlUserState userstate = otrl_userstate_create();

        QString path = config->saveLocation() + handler->context().accountId + QLatin1String("_privkeys");
        otrl_privkey_read(userstate, path.toLocal8Bit());

        path = config->saveLocation() + handler->context().accountId + QLatin1String("_fingerprints");
        otrl_privkey_read_fingerprints(userstate, path.toLocal8Bit(), NULL, NULL);

        path = config->saveLocation() + handler->context().accountId + QLatin1String("_instags");
        otrl_instag_read(userstate, path.toLocal8Bit());

        UserStateBoxPtr usPtr(new UserStateBox(userstate));
        userStates.insert(handler->context().accountId, usPtr);
        return SessionPtr(new Session(handler, usPtr.data(), this));
    } else {
        return SessionPtr(new Session(handler, usIt->data(), this));
    }
}

} /* namespace OTR */
