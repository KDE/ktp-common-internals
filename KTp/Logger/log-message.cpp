/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "log-message.h"
#include "message-private.h"

using namespace KTp;

LogMessage::LogMessage(const LogEntity &from, const Tp::AccountPtr &account,
                       const QDateTime &dt, const QString &message,
                       const QString &messageToken):
    Message(new KTp::Message::Private)
{
    d->senderId = from.id();
    d->senderAlias = from.alias();
    d->isHistory = true;
    d->messageType = Tp::ChannelTextMessageTypeNormal;
    d->sentTime = dt;
    d->token = messageToken;

    setMainMessagePart(message);

    if (account->normalizedName() == senderId()) {
        d->direction = KTp::Message::LocalToRemote;
    } else {
        d->direction = KTp::Message::RemoteToLocal;
    }
}

LogMessage::LogMessage(const LogMessage& other):
    Message(other)
{
}

LogMessage::~LogMessage()
{
}
