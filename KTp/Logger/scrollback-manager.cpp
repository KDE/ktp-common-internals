/*
    Copyright (C) 2011  Dominik Schmidt <kde@dominik-schmidt.de>
    Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "scrollback-manager.h"

#include "../message-processor.h"
#include "log-entity.h"
#include "log-manager.h"
#include "pending-logger-dates.h"
#include "pending-logger-logs.h"

#include "debug.h"

#include <TelepathyQt/Types>
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>

class ScrollbackManager::Private
{
  public:
    Private(): scrollbackLength(10)
    {
    }

    Tp::AccountPtr account;
    Tp::TextChannelPtr textChannel;
    KTp::LogEntity contactEntity;
    int scrollbackLength;
    QList<QDate> datesCache;
    QList<KTp::LogMessage> messagesCache;
    QString fromMessageToken;
};

ScrollbackManager::ScrollbackManager(QObject *parent)
    : QObject(parent),
    d(new Private)
{
}

ScrollbackManager::~ScrollbackManager()
{
    delete d;
}

bool ScrollbackManager::exists() const
{
    if (d->account.isNull() || d->textChannel.isNull() ) {
        return false;
    }

    return KTp::LogManager::instance()->logsExist(d->account, d->contactEntity);
}

void ScrollbackManager::setTextChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &textChannel)
{
    d->textChannel = textChannel;
    d->account = account;

    if (d->account.isNull() || d->textChannel.isNull()) {
        return;
    }

    KTp::LogEntity contactEntity;
    if (d->textChannel->targetHandleType() == Tp::HandleTypeContact) {
        d->contactEntity = KTp::LogEntity(d->textChannel->targetHandleType(),
                                       d->textChannel->targetContact()->id(),
                                       d->textChannel->targetContact()->alias());
    } else if (d->textChannel->targetHandleType() == Tp::HandleTypeRoom) {
        d->contactEntity = KTp::LogEntity(d->textChannel->targetHandleType(),
                                       d->textChannel->targetId());
    }
}

void ScrollbackManager::setScrollbackLength(int n)
{
    d->scrollbackLength = n;
}

int ScrollbackManager::scrollbackLength() const
{
    return d->scrollbackLength;
}

void ScrollbackManager::fetchScrollback()
{
    fetchHistory(d->scrollbackLength);
}

void ScrollbackManager::fetchHistory(int n, const QString &fromMessageToken)
{
    if (n > 0 && !d->account.isNull() && !d->textChannel.isNull()) {
        if (d->contactEntity.isValid()) {
            d->fromMessageToken = fromMessageToken;
            KTp::LogManager *manager = KTp::LogManager::instance();
            KTp::PendingLoggerDates *dates = manager->queryDates(d->account, d->contactEntity);
            connect(dates, SIGNAL(finished(KTp::PendingLoggerOperation*)),
                    this, SLOT(onDatesFinished(KTp::PendingLoggerOperation*)));
            return;
        }
    }

    //in all other cases finish immediately.
    QList<KTp::Message> messages;
    Q_EMIT fetched(messages);
}

void ScrollbackManager::onDatesFinished(KTp::PendingLoggerOperation* po)
{
    KTp::PendingLoggerDates *datesOp = qobject_cast<KTp::PendingLoggerDates*>(po);
    if (datesOp->hasError()) {
        qCWarning(KTP_LOGGER) << "Failed to fetch dates:" << datesOp->error();
        Q_EMIT fetched(QList<KTp::Message>());
        return;
    }

    const QList<QDate> dates = datesOp->dates();
    if (dates.isEmpty()) {
        Q_EMIT fetched(QList<KTp::Message>());
        return;
    }


    // Store all the fetched dates for later reusing
    d->datesCache = dates;

    // Request all logs for the last date in the datesCache and remove
    // it from the cache
    KTp::LogManager *manager = KTp::LogManager::instance();
    KTp::PendingLoggerLogs *logs = manager->queryLogs(datesOp->account(), datesOp->entity(),
                                                      d->datesCache.takeLast());
    connect(logs, SIGNAL(finished(KTp::PendingLoggerOperation*)),
            this, SLOT(onEventsFinished(KTp::PendingLoggerOperation*)));
}

void ScrollbackManager::onEventsFinished(KTp::PendingLoggerOperation *op)
{
    KTp::PendingLoggerLogs *logsOp = qobject_cast<KTp::PendingLoggerLogs*>(op);
    if (logsOp->hasError()) {
        qCWarning(KTP_LOGGER) << "Failed to fetch events:" << logsOp->error();
        Q_EMIT fetched(QList<KTp::Message>());
        return;
    }

    QStringList queuedMessageTokens;
    if (!d->textChannel.isNull()) {
        Q_FOREACH(const Tp::ReceivedMessage &message, d->textChannel->messageQueue()) {
            queuedMessageTokens.append(message.messageToken());
        }
    }

    // get last n (d->fetchLast) messages that are not queued
    QList<KTp::LogMessage> allMessages = logsOp->logs();

    // First of all check if the "fromMessageToken" was specified and if yes,
    // then look if any message in the current fetched logs set
    // contains the requested message that the logs should start from
    // In case the token is empty, it compares message date and message body
    if (!d->fromMessageToken.isEmpty()) {
        int i = 0;
        for (i = 0; i < allMessages.size(); i++) {
            if (allMessages.at(i).token().isEmpty()) {
                const QString token = allMessages.at(i).time().toString(Qt::ISODate) + allMessages.at(i).mainMessagePart();
                if (token == d->fromMessageToken) {
                    break;
                }
            } else {
                if (allMessages.at(i).token() == d->fromMessageToken) {
                    break;
                }
            }
        }

        // If the message with the "fromMessageToken" is in the current logs set,
        // drop all the messages beyond the one with "fromMessageToken" as we don't
        // care about those. If the message is not in this set, clear all the fetched
        // messages set (the querying works backwards from the most recent dates; so
        // if this set does not contain the message with the specified token, it will
        // be in some older set and all these newer messages than the "fromMessageToken"
        // message are useless)
        if (i < allMessages.size()) {
            allMessages = allMessages.mid(0, i);

            // Clear the fromMessageToken to not break fetching more logs from history.
            // For example: the current set has the requested-token message but has
            // only 3 messages, so below it queries further in the history to fetch
            // more messages. If the token would not be empty when the new request finishes,
            // it would discard the fetched messages in the following else branch
            // resulting in no messages being fetched
            d->fromMessageToken.clear();
        } else {
            allMessages.clear();
        }
    }

    // The messages are fetched backwards - the most recent
    // date is fetched first, so take the messages from the previous
    // dates and append them to the messages from the current date,
    // that will sort them with newest dates first.
    // This is useful only for the case below when the fetched messages
    // are less than the requested scrollback length
    allMessages.append(d->messagesCache);

    // If the logs for the last date were too few, cache the
    // retrieved messages and request logs from another date
    if (allMessages.size() < d->scrollbackLength) {
        // Only request more logs when there are more dates to query
        if (!d->datesCache.isEmpty()) {
            d->messagesCache = allMessages;

            KTp::LogManager *manager = KTp::LogManager::instance();
            KTp::PendingLoggerLogs *logs = manager->queryLogs(logsOp->account(), logsOp->entity(),
                                                            d->datesCache.takeLast());
            connect(logs, SIGNAL(finished(KTp::PendingLoggerOperation*)),
                    this, SLOT(onEventsFinished(KTp::PendingLoggerOperation*)));

            return;
        }
    }

    QList<KTp::Message> messages;
    const KTp::MessageContext ctx(d->account, d->textChannel);
    for (int i = qMax(allMessages.count() - d->scrollbackLength, 0) ; i < allMessages.count(); ++i) {
        const KTp::LogMessage message = allMessages[i];
        if (queuedMessageTokens.contains(message.token())) {
            continue;
        }

        messages << KTp::MessageProcessor::instance()->processIncomingMessage(message, ctx);
    }

    d->messagesCache.clear();
    d->datesCache.clear();

    Q_EMIT fetched(messages);
}
