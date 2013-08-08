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

#include "pending-logger-logs.h"

using namespace KTp;

class PendingLoggerLogs::Private
{
  public:
    Private(const Tp::AccountPtr &account_, const KTp::LogEntity &entity_, const QDate &date_):
        account(account_),
        entity(entity_),
        date(date_)
    {
    }

    Tp::AccountPtr account;
    KTp::LogEntity entity;
    QDate date;
    QList<KTp::LogMessage> logs;
};

PendingLoggerLogs::PendingLoggerLogs(const Tp::AccountPtr &account,
                                     const KTp::LogEntity &entity,
                                     const QDate &date,
                                     QObject* parent):
    PendingLoggerOperation(parent),
    d(new Private(account, entity, date))
{
}

PendingLoggerLogs::~PendingLoggerLogs()
{
    delete d;
}

Tp::AccountPtr PendingLoggerLogs::account() const
{
    return d->account;
}

LogEntity PendingLoggerLogs::entity() const
{
    return d->entity;
}

QDate PendingLoggerLogs::date() const
{
    return d->date;
}

QList<KTp::LogMessage> PendingLoggerLogs::logs() const
{
    return d->logs;
}

void PendingLoggerLogs::appendLogs(const QList<LogMessage> &logs)
{
    d->logs.append(logs);
}
