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

#include "pending-tp-logger-dates.h"

#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/PendingDates>
#include <TelepathyQt/Contact>

PendingTpLoggerDates::PendingTpLoggerDates(const Tp::AccountPtr &account,
                                           const Tp::ContactPtr &contact,
                                           QObject* parent):
    PendingLoggerDates(account, contact, parent)
{
    // FIXME: Handle rooms
    Tpl::EntityPtr entity = Tpl::Entity::create(contact->id().toLatin1().constData(),
                                                Tpl::EntityTypeContact,
                                                contact->alias().toLatin1().constData(),
                                                0);

    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tpl::PendingDates *dates = manager->queryDates(account, entity, Tpl::EventTypeMaskText);
    connect(dates, SIGNAL(finished(Tpl::PendingOperation*)),
            this, SLOT(datesRetrieved(Tpl::PendingOperation*)));
}

PendingTpLoggerDates::~PendingTpLoggerDates()
{
}

void PendingTpLoggerDates::datesRetrieved(Tpl::PendingOperation *op)
{
    Tpl::PendingDates *pd = qobject_cast<Tpl::PendingDates*>(op);
    Q_ASSERT(pd);

    if (pd->isError()) {
        setError(pd->errorName() + QLatin1String(": ") + pd->errorMessage());
        emitFinished();
        return;
    }

    setDates(pd->dates());
    emitFinished();
}
