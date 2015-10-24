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

#include "otr-message.h"

#include "KTp/OTR/constants.h"

namespace OTR
{
    Message::Message()
        : dir(MessageDirection::TO_PEER)
    {
        message << Tp::MessagePart() << Tp::MessagePart();
        setType(Tp::ChannelTextMessageTypeNormal);
    }

    Message::Message(const Tp::MessagePartList &message)
        : dir(MessageDirection::TO_PEER),
        message(message)
    {
        while(this->message.size() < 2) {
            this->message << Tp::MessagePart();
        }
        if(!this->message[0].contains(QLatin1String("message-type"))) {
            setType(Tp::ChannelTextMessageTypeNormal);
        }
    }

    const Tp::MessagePartList& Message::parts() const
    {
        return message;
    }

    QString Message::text() const
    {
        auto it = message[1].find(QLatin1String("content"));
        if(it == message[1].end()) {
            return QLatin1String("");
        } else {
            return it->variant().toString();
        }
    }

    void Message::setText(const QString &text, const QString &contentType)
    {
        message[1].insert(QLatin1String("content-type"),
                QDBusVariant(contentType));
        message[1].insert(QLatin1String("content"), QDBusVariant(text));
    }

    QString Message::contentType() const
    {
        auto it = message[1].find(QLatin1String("content-type"));
        if(it == message[1].end()) {
            return QLatin1String("");
        } else {
            return it->variant().toString();
        }
    }

    void Message::setType(Tp::ChannelTextMessageType msgType)
    {
        message[0].insert(QLatin1String("message-type"), QDBusVariant(msgType));
    }

    Tp::ChannelTextMessageType Message::type() const
    {
        return static_cast<Tp::ChannelTextMessageType>(message[0][QLatin1String("message-type")].variant().toUInt(nullptr));
    }

    bool Message::isOTRmessage() const
    {
        return otrl_proto_message_type(text().toLocal8Bit()) != OTRL_MSGTYPE_NOTOTR;
    }

    MessageDirection Message::direction() const
    {
        return dir;
    }

    void Message::setDirection(MessageDirection direction)
    {
        dir = direction;
    }

    bool Message::isOTRevent() const
    {
        return message[0].contains(OTR_MESSAGE_EVENT_HEADER);
    }

    void Message::setOTRevent(OtrlMessageEvent msgEvent)
    {
        message[0].insert(OTR_MESSAGE_EVENT_HEADER, QDBusVariant(static_cast<uint>(msgEvent)));
    }

    OtrlMessageEvent Message::getOTRevent() const
    {
        if(isOTRevent()) {
            return static_cast<OtrlMessageEvent>(message[0][OTR_MESSAGE_EVENT_HEADER].variant().toUInt(nullptr));
        } else {
            return OTRL_MSGEVENT_NONE;
        }
    }

    void Message::setOTRheader(const QString &header, const QString &text)
    {
        message[0].insert(header, QDBusVariant(text));
    }

    QString Message::getOTRheader(const QString &header)
    {
        auto it = message[0].find(header);
        if(it == message[0].end()) {
            return QLatin1String("");
        } else {
            return it->variant().toString();
        }
    }

    void Message::setTimestamp(qint64 timestamp)
    {
        message[0].insert(QLatin1String("message-sent"), QDBusVariant(timestamp));
    }

    qint64 Message::getTimestamp() const
    {
        auto it = message[0].find(QLatin1String("message-sent"));
        if(it == message[0].end()) {
            return 0;
        } else {
            return it->variant().toLongLong(NULL);
        }
    }

    void Message::setSenderId(const QString &senderId)
    {
        message[0].insert(QLatin1String("message-sender-id"), QDBusVariant(senderId));
    }

    QString Message::getSenderId() const
    {
        auto it = message[0].find(QLatin1String("message-sender-id"));
        if(it == message[0].end()) {
            return QLatin1String("");
        } else {
            return it->variant().toString();
        }
    }

    void Message::setSender(uint sender)
    {
        message[0].insert(QLatin1String("message-sender"), QDBusVariant(sender));
    }

    uint Message::getSender() const
    {
        auto it = message[0].find(QLatin1String("message-sender"));
        if(it == message[0].end()) {
            return 0;
        } else {
            return it->variant().toUInt(NULL);
        }
    }

    void Message::setToken(const QString &token)
    {
        message[0].insert(QLatin1String("message-token"), QDBusVariant(token));
    }

    QString Message::getToken() const
    {
        auto it = message[0].find(QLatin1String("message-sender"));
        if(it == message[0].end()) {
            return QLatin1String("");
        } else {
            return it->variant().toString();
        }
    }

} /* namespace OTR */
