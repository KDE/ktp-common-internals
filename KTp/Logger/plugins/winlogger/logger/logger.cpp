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

#include "logger.h"
#include "db.h"
#include <log-entity.h>

#include <TelepathyQt/Channel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Account>
#include <KDebug>

Logger::Logger(const Tp::AccountPtr &account,
               const Tp::ConnectionPtr &connection,
               const Tp::ChannelPtr &channel):
    QObject(),
    mAccount(account),
    mConnection(connection),
    mChannel(Tp::TextChannelPtr::dynamicCast<Tp::Channel>(channel))
{
    kDebug() << "Logging channel" << channel.constData() << "to" << channel->targetId();

    if (mChannel->targetHandleType() == Tp::HandleTypeContact) {
        mRemoteEntity = KTp::LogEntity(KTp::LogEntity::EntityTypeContact,
                            mChannel->targetContact()->id(),
                            mChannel->targetContact()->alias());
    } else if (mChannel->targetHandleType() == Tp::HandleTypeRoom) {
        mRemoteEntity = KTp::LogEntity(KTp::LogEntity::EntityTypeRoom,
                            mChannel->targetId());
    }

    connect(mChannel.constData(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            this, SLOT(onChannelInvalidated(Tp::DBusProxy*,QString,QString)));
    connect(mChannel.constData(), SIGNAL(messageReceived(Tp::ReceivedMessage)),
            this, SLOT(onMessageReceived(Tp::ReceivedMessage)));
    connect(mChannel.constData(), SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
            this, SLOT(onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString)));

    // Store pending messages
    Q_FOREACH (const Tp::ReceivedMessage &msg, mChannel->messageQueue()) {
        onMessageReceived(msg);
    }
}

Logger::~Logger()
{
}

void Logger::onChannelInvalidated(Tp::DBusProxy *dbusProxy, const QString &errorName,
                                  const QString &errorMessage)
{
    kDebug() << "Channel" << mChannel.constData() << "invalidated";
    kDebug() << "Error:" << errorName;
    kDebug() << "Error message:" << errorMessage;

    deleteLater();
}

void Logger::onMessageReceived(const Tp::ReceivedMessage &message)
{
    Db::instance()->logMessage(mAccount->uniqueIdentifier(),
                               mRemoteEntity, message, false);
}

void Logger::onMessageSent(const Tp::Message &message, Tp::MessageSendingFlags flags,
                           const QString &sentMessageToken)
{
    Db::instance()->logMessage(mAccount->uniqueIdentifier(),
                               mRemoteEntity, message, true);
}
