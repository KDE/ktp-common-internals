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

#ifndef KTP_PROXY_OTR_UTILS_HEADER
#define KTP_PROXY_OTR_UTILS_HEADER

#include "otr-constants.h"

#include <QString>
#include <QStringList>
#include <QDBusObjectPath>

#include <KDebug>

extern "C" {
#include <libotr/privkey.h>
}

namespace OTR {

    struct SessionContext
    {
        const QString accountId;
        const QString accountName;
        const QString recipientName;
        const QString protocol;
    };

namespace utils
{
    inline QString humanReadable(const unsigned char fingerprint[20])
    {
        char human[OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
        otrl_privkey_hash_to_human(human, fingerprint);
        return QString::fromLocal8Bit(human, OTRL_PRIVKEY_FPRINT_HUMAN_LEN-1);
    }

    inline QString accountIdFor(const QDBusObjectPath &objectPath)
    {
        QStringList elems = objectPath.path().split(QLatin1Char('/'));
        return QStringList(elems.mid(elems.size()-3)).join(QLatin1String("."));
    }

    inline QString accountIdFor(const QString &cmName, const QString &protocol, const QString &acc)
    {
        return cmName + QLatin1Char('.') + protocol + QLatin1Char('.') + acc;
    }

    inline QString cmNameFromAccountId(const QString &accountId)
    {
        return accountId.split(QLatin1Char('.'))[0];
    }

    inline QString protocolFromAccountId(const QString &accountId)
    {
        return accountId.split(QLatin1Char('.'))[1];
    }

    inline QString accFromAccountId(const QString &accountId)
    {
        return accountId.split(QLatin1Char('.'))[2];
    }

    inline QDBusObjectPath objectPathFor(const QString &accountId)
    {
        return QDBusObjectPath(
                QLatin1String("/org/freedesktop/Telepathy/Account/") +
                cmNameFromAccountId(accountId) + QLatin1Char('/') +
                protocolFromAccountId(accountId) + QLatin1Char('/') +
                accFromAccountId(accountId));
    }

    inline TrustLevel getTrustLevel(const SessionContext &ctx, OtrlUserState userState, otrl_instag_t instance)
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
}
}

#endif
