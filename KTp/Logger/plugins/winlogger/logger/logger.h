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

#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore/QObject>
#include <TelepathyQt/Types>
#include <TelepathyQt/ReceivedMessage>

#include <KTp/Logger/log-entity.h>

class Logger : public QObject
{
    Q_OBJECT

  public:
    explicit Logger(const Tp::AccountPtr &account, const Tp::ConnectionPtr &connection,
                    const Tp::ChannelPtr &channel);
    ~Logger();

  private Q_SLOTS:
    void onChannelInvalidated(Tp::DBusProxy *dbusProxy, const QString &errorName, const QString &errorMessage);
    void onMessageReceived(const Tp::ReceivedMessage &message);
    void onMessageSent(const Tp::Message &message, Tp::MessageSendingFlags flags,
                       const QString &sentMessageToken);

  private:
    KTp::LogEntity mRemoteEntity;
    Tp::AccountPtr mAccount;
    Tp::ConnectionPtr mConnection;
    Tp::TextChannelPtr mChannel;
};

#endif // LOGGER_H
