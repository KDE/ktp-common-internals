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

using namespace KTp;

class PendingLoggerDates::Private
{
  public:
    Private(const Tp::AccountPtr &account_, const Tp::ContactPtr &contact_):
        account(account_),
        contact(contact_)
    {
    }

    Tp::AccountPtr account;
    Tp::ContactPtr contact;
    QList<QDate> dates;
};

PendingLoggerDates::PendingLoggerDates(const Tp::AccountPtr& account,
                                       const Tp::ContactPtr& contact,
                                       QObject *parent) :
    PendingLoggerOperation(parent),
    d(new Private(account, contact))
{
}

PendingLoggerDates::~PendingLoggerDates()
{
    delete d;
}

QList<QDate> PendingLoggerDates::dates() const
{
    return d->dates;
}

Tp::AccountPtr PendingLoggerDates::account() const
{
    return d->account;
}

Tp::ContactPtr PendingLoggerDates::contact() const
{
    return d->contact;
}
