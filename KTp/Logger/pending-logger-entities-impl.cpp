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

#include "pending-logger-entities-impl.h"
#include "abstract-logger-plugin.h"
#include "debug.h"

PendingLoggerEntitiesImpl::PendingLoggerEntitiesImpl(const Tp::AccountPtr &account,
                                                     QObject* parent):
    PendingLoggerEntities(account, parent)
{
    if (plugins().isEmpty()) {
        emitFinished();
        return;
    }

    Q_FOREACH (KTp::AbstractLoggerPlugin *plugin, plugins()) {
        if (!plugin->handlesAccount(account)) {
            continue;
        }

        PendingLoggerOperation *op = plugin->queryEntities(account);
        if (!op) {
            continue;
        }

        connect(op, SIGNAL(finished(KTp::PendingLoggerOperation*)),
                this, SLOT(operationFinished(KTp::PendingLoggerOperation*)));
        mRunningOps << op;
    }
}

PendingLoggerEntitiesImpl::~PendingLoggerEntitiesImpl()
{
}

void PendingLoggerEntitiesImpl::operationFinished(KTp::PendingLoggerOperation* op)
{
    Q_ASSERT(mRunningOps.contains(op));
    mRunningOps.removeAll(op);

    KTp::PendingLoggerEntities *operation = qobject_cast<KTp::PendingLoggerEntities*>(op);
    Q_ASSERT(operation);

    const QList<KTp::LogEntity> newEntities = operation->entities();
    qCDebug(KTP_LOGGER) << "Plugin" << op->parent() << "returned" << newEntities.count() << "entities";
    Q_FOREACH (const KTp::LogEntity &entity, newEntities) {
        if (!entities().contains(entity)) {
            appendEntity(entity);
        }
    }

    if (mRunningOps.isEmpty()) {
        emitFinished();
    }
}
