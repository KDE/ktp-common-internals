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

#include "log-search-hit.h"

#include <QtCore/QSharedData>

#include <TelepathyQt/Account>

#include "log-entity.h"

using namespace KTp;

class LogSearchHit::Private: public QSharedData
{
  public:
    Private(const Tp::AccountPtr &account_, const KTp::LogEntity &entity_,
            const QDate &date_):
        QSharedData(),
        account(account_),
        entity(entity_),
        date(date_)
    {
    }

    Private(const Private &other):
        QSharedData(other),
        account(other.account),
        entity(other.entity),
        date(other.date)
    {
    }

    Tp::AccountPtr account;
    KTp::LogEntity entity;
    QDate date;
};

LogSearchHit::LogSearchHit(const Tp::AccountPtr &account, const LogEntity &entity,
                           const QDate &date):
    d(new Private(account, entity, date))
{
}

LogSearchHit::LogSearchHit(const LogSearchHit& other):
    d(other.d)
{
}

LogSearchHit::~LogSearchHit()
{
}

LogSearchHit& LogSearchHit::operator=(const LogSearchHit &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

Tp::AccountPtr LogSearchHit::account() const
{
    return d->account;
}

KTp::LogEntity LogSearchHit::entity() const
{
    return d->entity;
}

QDate LogSearchHit::date() const
{
    return d->date;
}
