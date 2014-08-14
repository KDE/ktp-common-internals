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

#ifndef KTP_PROXY_OTR_VALUE_TYPES_HEADER
#define KTP_PROXY_OTR_VALUE_TYPES_HEADER

#include "otr-constants.h"

#include <TelepathyQt/Message>

extern "C" {
#include <libotr/privkey.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
}

namespace OTR
{

    class Message
    {
        public:
            Message();
            Message(const Tp::MessagePartList &message);

            const Tp::MessagePartList& parts() const;

            QString text() const;
            void setText(const QString &text, const QString &contentType = QLatin1String("text/plain"));
            QString contentType() const;

            void setType(Tp::ChannelTextMessageType msgType);
            Tp::ChannelTextMessageType type() const;

            /** returns true if it is OTR protocol specific message */
            bool isOTRmessage() const;

            MessageDirection direction() const;
            void setDirection(MessageDirection direction);

            bool isOTRevent() const;
            void setOTRevent(OtrlMessageEvent msgEvent);
            OtrlMessageEvent getOTRevent() const;

            void setOTRheader(const QString &header, const QString &text);
            QString getOTRheader(const QString &header);

            void setTimestamp(qint64 timestamp);
            qint64 getTimestamp() const;

            void setSenderId(const QString &senderId);
            QString getSenderId() const;

            void setSender(uint sender);
            uint getSender() const;

            void setToken(const QString &token);
            QString getToken() const;

        private:
            MessageDirection dir;
            Tp::MessagePartList message;
    };

} /* namespace OTR */

#endif
