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

#include "pending-tp-logger-search.h"

#include <TelepathyLoggerQt/LogManager>
#include <TelepathyLoggerQt/PendingSearch>
#include <TelepathyLoggerQt/SearchHit>
#include <TelepathyLoggerQt/Entity>

#include <TelepathyQt/Account>

#include "log-search-hit.h"
#include "utils.h"

PendingTpLoggerSearch::PendingTpLoggerSearch(const QString& term, QObject* parent):
    PendingLoggerSearch(term, parent)
{
    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tpl::PendingSearch *search = manager->search(term, Tpl::EventTypeMaskText);
    connect(search, SIGNAL(finished(Tpl::PendingOperation*)),
            this, SLOT(searchFinished(Tpl::PendingOperation*)));
}

PendingTpLoggerSearch::~PendingTpLoggerSearch()
{

}

void PendingTpLoggerSearch::searchFinished(Tpl::PendingOperation* op)
{
    Tpl::PendingSearch *search = qobject_cast<Tpl::PendingSearch*>(op);
    const Tpl::SearchHitList hits = search->hits();

    Q_FOREACH (const Tpl::SearchHit &hit, hits) {
        appendSearchHit(KTp::LogSearchHit(hit.account(), Utils::fromTplEntity(hit.target()), hit.date()));
    }

    emitFinished();
}
