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

#include "pending-logger-dates-impl.h"
#include "abstract-logger-plugin.h"

PendingLoggerDatesImpl::PendingLoggerDatesImpl(const Tp::AccountPtr &account,
                                               const Tp::ContactPtr &contact,
                                               QObject* parent):
    PendingLoggerDates(account, contact, parent)
{
    Q_FOREACH (KTp::AbstractLoggerPlugin *plugin, plugins()) {
        if (!plugin->handlesAccount(account)) {
            continue;
        }

        PendingLoggerOperation *op = plugin->queryDates(account, contact);
        if (!op) {
            continue;
        }

        connect(op, SIGNAL(finished(PendingLoggerOperation*)),
                this, SLOT(operationFinished(KTp::PendingLoggerOperation*)));
        mRunningOps << op;
    }
}

PendingLoggerDatesImpl::~PendingLoggerDatesImpl()
{
}

void PendingLoggerDatesImpl::operationFinished(KTp::PendingLoggerOperation *op)
{
    Q_ASSERT(mRunningOps.contains(op));
    mRunningOps.removeAll(op);

    KTp::PendingLoggerDates *operation = qobject_cast<KTp::PendingLoggerDates*>(op);
    Q_ASSERT(operation);

    const QList<QDate> newDates = operation->dates();
    QList<QDate> existingDates = dates();
    Q_FOREACH (const QDate &date, newDates) {
        if (!existingDates.contains(date)) {
            existingDates << date;
        }
    }

    setDates(existingDates);

    if (mRunningOps.isEmpty()) {
        emitFinished();
    }
}