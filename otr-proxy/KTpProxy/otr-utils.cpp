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

#include "otr-utils.h"

namespace OTR
{
namespace utils
{

    Fingerprint* findFingerprint(OtrlUserState userState, const QString &fp, const QString &user)
    {
        Fingerprint *fingerprint = nullptr;

        for(ConnContext *context = userState->context_root;
                context != nullptr; context = context->next)
        {
            if(user != QLatin1String(context->username)) {
                continue;
            }
            for(fingerprint = context->fingerprint_root.next;
                    fingerprint != nullptr; fingerprint = fingerprint->next)
            {
                if(humanReadable(fingerprint->fingerprint) == fp) {
                    return fingerprint;
                }
            }
        }

        return fingerprint;
    }

    bool isFingerprintInUse(Fingerprint *fingerprint)
    {
        // check if used in all parent contexts
        for(ConnContext *ctx_iter = fingerprint->context->m_context;
                ctx_iter != nullptr && ctx_iter->m_context == fingerprint->context->m_context;
                ctx_iter = ctx_iter->next)
        {
            if(ctx_iter->active_fingerprint == fingerprint) {
                return true;
            }
        }

        return false;
    }

    TrustLevel getTrustLevel(const SessionContext &ctx, OtrlUserState userState, otrl_instag_t instance)
    {
        ConnContext *context = otrl_context_find(
                userState,
                ctx.recipientName.toLocal8Bit(),
                ctx.accountName.toLocal8Bit(),
                ctx.protocol.toLocal8Bit(),
                instance, 0, NULL, NULL, NULL);

        if(context == nullptr) {
            kWarning() << "Could not get trust level";
            return TrustLevel::NOT_PRIVATE;
        }

        switch(context->msgstate) {
            case OTRL_MSGSTATE_PLAINTEXT:
                return TrustLevel::NOT_PRIVATE;
            case OTRL_MSGSTATE_ENCRYPTED:
                {
                    if(otrl_context_is_fingerprint_trusted(context->active_fingerprint)) {
                        return TrustLevel::VERIFIED;
                    } else {
                        return TrustLevel::UNVERIFIED;
                    }
                }
            case OTRL_MSGSTATE_FINISHED:
                return TrustLevel::FINISHED;
        }
        return TrustLevel::NOT_PRIVATE;
    }

} /* namespace utils */
} /* namespace OTR */
