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

#include "pending-logger-search.h"

using namespace KTp;

class PendingLoggerSearch::Private
{
  public:
    Private(const QString &term_):
        term(term_)
    {
    }

    QString term;
    QList<KTp::LogSearchHit> searchHits;
};

PendingLoggerSearch::PendingLoggerSearch(const QString &term, QObject *parent):
    PendingLoggerOperation(parent),
    d(new Private(term))
{
}

PendingLoggerSearch::~PendingLoggerSearch()
{
    delete d;
}

QString PendingLoggerSearch::term() const
{
    return d->term;
}

QList<KTp::LogSearchHit> PendingLoggerSearch::searchHits() const
{
    return d->searchHits;
}

void PendingLoggerSearch::appendSearchHit(const KTp::LogSearchHit &searchHit)
{
    d->searchHits << searchHit;
}

void PendingLoggerSearch::appendSearchHits(const QList<LogSearchHit> &searchHits)
{
    d->searchHits << searchHits;
}
