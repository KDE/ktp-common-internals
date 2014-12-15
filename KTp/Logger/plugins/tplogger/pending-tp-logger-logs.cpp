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

#include "pending-tp-logger-logs.h"
#include "utils.h"

#include <TelepathyQt/Contact>
#include <TelepathyLoggerQt/LogManager>
#include <TelepathyLoggerQt/Entity>
#include <TelepathyLoggerQt/TextEvent>
#include <TelepathyLoggerQt/PendingEvents>

#include <KTp/message-processor.h>

#include "Logger/debug.h"

PendingTpLoggerLogs::PendingTpLoggerLogs(const Tp::AccountPtr &account,
                                         const KTp::LogEntity &entity,
                                         const QDate &date,
                                         QObject *parent):
    PendingLoggerLogs(account, entity, date, parent)
{
    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tpl::PendingEvents *events = manager->queryEvents(account, Utils::toTplEntity(entity),
                                                      Tpl::EventTypeMaskText, date);
    connect(events, SIGNAL(finished(Tpl::PendingOperation*)),
            this, SLOT(logsRetrieved(Tpl::PendingOperation*)));
}

PendingTpLoggerLogs::~PendingTpLoggerLogs()
{
}

void PendingTpLoggerLogs::logsRetrieved(Tpl::PendingOperation *op)
{
    Tpl::PendingEvents *pe = qobject_cast<Tpl::PendingEvents*>(op);
    Q_ASSERT(pe);

    if (pe->isError()) {
        setError(pe->errorName() + QLatin1String(": ") + pe->errorMessage());
        emitFinished();
        return;
    }

    QList<Tpl::EventPtr> events = pe->events();
    QList<KTp::LogMessage> logs;
    Q_FOREACH (const Tpl::EventPtr &event, events) {
        const Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();
        if (textEvent.isNull()) {
            qWarning() << "Received a null TextEvent!";
            continue;
        }

        logs << KTp::LogMessage(Utils::fromTplEntity(event->sender()),
                                account(), event->timestamp(), textEvent->message(),
                                textEvent->messageToken());
    }

    appendLogs(logs);
    emitFinished();
}

