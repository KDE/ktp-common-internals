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

#include <QString>

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

        return QString::fromLocal8Bit(human, OTRL_PRIVKEY_FPRINT_HUMAN_LEN);
    }
}
}

#endif
