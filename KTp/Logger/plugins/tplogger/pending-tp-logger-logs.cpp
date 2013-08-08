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

#include <TelepathyQt/Contact>
#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/TextEvent>
#include <TelepathyLoggerQt4/PendingEvents>
#include <KTp/message-processor.h>

PendingTpLoggerLogs::PendingTpLoggerLogs(const Tp::AccountPtr &account,
                                         const Tp::ContactPtr &contact,
                                         const QDate &date,
                                         QObject *parent):
    PendingLoggerLogs(account, contact, date, parent)
{
    // FIXME: Handle rooms
    Tpl::EntityPtr entity = Tpl::Entity::create(contact->id().toLatin1().constData(),
                                                Tpl::EntityTypeContact,
                                                contact->alias().toLatin1().constData(),
                                                0);

    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tpl::PendingEvents *events = manager->queryEvents(account, entity, Tpl::EventTypeMaskText, date);
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
        logs << KTp::LogMessage(KTp::LogEntity(
                        event->sender()->entityType() == Tpl::EntityTypeContact ?
                            KTp::LogEntity::EntityTypeContact : KTp::LogEntity::EntityTypeRoom,
                            event->sender()->identifier(), event->sender()->alias()),
                        event->account(), event->timestamp(), textEvent->message());
    }

    appendLogs(logs);
    emitFinished();
}

