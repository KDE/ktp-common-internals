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

#include "otr-message.h"

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

    void Message::setText(const QString &text)
    {
        // FIXME - using only text/plan content-type, what about html?
        message[1].insert(QLatin1String("content-type"),
                QDBusVariant(QLatin1String("text/plain")));
        message[1].insert(QLatin1String("content"), QDBusVariant(text));
    }

    void Message::setType(Tp::ChannelTextMessageType msgType)
    {
        message[0].insert(QLatin1String("message-type"), QDBusVariant(msgType));
    }

    Tp::ChannelTextMessageType Message::type() const
    {
        return static_cast<Tp::ChannelTextMessageType>(message[0]["message-type"].variant().toUInt(NULL));
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
        return message[0].contains(QLatin1String("otr-message-event"));
    }

    void Message::setOTRevent(OtrlMessageEvent msgEvent)
    {
        message[0].insert(QLatin1String("otr-message-event"), QDBusVariant(static_cast<uint>(msgEvent)));
    }

    OtrlMessageEvent Message::getOTRevent() const
    {
        return static_cast<OtrlMessageEvent>(message[0][QLatin1String("otr-message-event")].variant().toUInt(NULL));
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

} /* namespace OTR */
