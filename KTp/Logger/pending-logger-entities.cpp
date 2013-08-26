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

#include "pending-logger-entities.h"
#include "log-entity.h"

#include <TelepathyQt/Account>

using namespace KTp;

class PendingLoggerEntities::Private
{
  public:
    Private(const Tp::AccountPtr &account_):
        account(account_)
    {
    }

    Tp::AccountPtr account;
    QList<KTp::LogEntity> entities;
};

PendingLoggerEntities::PendingLoggerEntities(const Tp::AccountPtr &account,
                                             QObject *parent):
    PendingLoggerOperation(parent),
    d(new Private(account))
{
}

PendingLoggerEntities::~PendingLoggerEntities()
{
    delete d;
}

Tp::AccountPtr PendingLoggerEntities::account() const
{
    return d->account;
}

QList<KTp::LogEntity> PendingLoggerEntities::entities() const
{
    return d->entities;
}

void PendingLoggerEntities::appendEntities(const QList<LogEntity> &entities)
{
    d->entities << entities;
}

void PendingLoggerEntities::appendEntity(const LogEntity &entity)
{
    d->entities << entity;
}
