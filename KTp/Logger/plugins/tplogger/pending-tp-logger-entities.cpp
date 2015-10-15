/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "pending-tp-logger-entities.h"
#include "utils.h"

#include <TelepathyLoggerQt/LogManager>
#include <TelepathyLoggerQt/Entity>
#include <TelepathyLoggerQt/PendingEntities>
#include <TelepathyQt/Account>

PendingTpLoggerEntities::PendingTpLoggerEntities(const Tp::AccountPtr &account,
                                                 QObject *parent):
    PendingLoggerEntities(account, parent)
{
    Tpl::LogManagerPtr logManager = Tpl::LogManager::instance();
    Tpl::PendingEntities *op = logManager->queryEntities(account);
    connect(op, SIGNAL(finished(Tpl::PendingOperation*)),
            this, SLOT(entitiesRetrieved(Tpl::PendingOperation*)));
}

PendingTpLoggerEntities::~PendingTpLoggerEntities()
{
}

void PendingTpLoggerEntities::entitiesRetrieved(Tpl::PendingOperation *op)
{
    if (op->isError()) {
        setError(op->errorName() + QLatin1String(": ") + op->errorMessage());
        emitFinished();
        return;
    }

    Tpl::PendingEntities *pe = qobject_cast<Tpl::PendingEntities*>(op);
    Q_ASSERT(pe);

    Tpl::EntityPtrList entities = pe->entities();
    QList<KTp::LogEntity> logEntities;
    Q_FOREACH (const Tpl::EntityPtr entity, entities) {
        logEntities << Utils::fromTplEntity(entity);
    }

    appendEntities(logEntities);
    emitFinished();
}
