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

#include "pending-logger-dates.h"
#include "abstract-logger-plugin.h"
#include "log-entity.h"

#include <TelepathyQt/Account>
#include <TelepathyQt/Contact>

using namespace KTp;

class PendingLoggerDates::Private
{
  public:
    Private(const Tp::AccountPtr &account_, const KTp::LogEntity &entity_):
        account(account_),
        entity(entity_)
    {
    }

    Tp::AccountPtr account;
    KTp::LogEntity entity;
    QList<QDate> dates;
};

PendingLoggerDates::PendingLoggerDates(const Tp::AccountPtr &account,
                                       const KTp::LogEntity &entity,
                                       QObject *parent) :
    PendingLoggerOperation(parent),
    d(new Private(account, entity))
{
}

PendingLoggerDates::~PendingLoggerDates()
{
    delete d;
}

void PendingLoggerDates::setDates(const QList<QDate> &dates)
{
    d->dates = dates;
}

QList<QDate> PendingLoggerDates::dates() const
{
    return d->dates;
}

Tp::AccountPtr PendingLoggerDates::account() const
{
    return d->account;
}

LogEntity PendingLoggerDates::entity() const
{
    return d->entity;
}
