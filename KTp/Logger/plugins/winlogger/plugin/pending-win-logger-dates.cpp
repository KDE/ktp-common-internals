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

#include "pending-win-logger-dates.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <TelepathyQt/Account>

#include <KDebug>

PendingWinLoggerDates::PendingWinLoggerDates(const Tp::AccountPtr &account,
                                             const Tp::ContactPtr &contact,
                                             const QSqlDatabase &db,
                                             QObject *parent):
    PendingLoggerDates(account, contact, parent),
    mDb(db)
{
    QFuture<QList<QDate> > future = QtConcurrent::run(this, &PendingWinLoggerDates::runQuery);
    QFutureWatcher<QList<QDate> > *watcher = new QFutureWatcher<QList<QDate> >(this);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()),
            this, SLOT(queryFinished()));
}

PendingWinLoggerDates::~PendingWinLoggerDates()
{
}

QList<QDate> PendingWinLoggerDates::runQuery()
{
    QSqlQuery query(mDb);
    if (!query.prepare(QLatin1String("SELECT DISTINCT logs.timestamp FROM logs "
                                     "LEFT JOIN contacts ON logs.contactId = contacts.id "
                                     "LEFT JOIN accounts ON contacts.accountId = accounts.id "
                                     "WHERE accounts.uid = ? AND contacts.uid = ?"))) {
        kWarning() << query.lastError().text();
        return QList<QDate>();
    }

    query.addBindValue(account()->uniqueIdentifier());
    query.addBindValue(contact()->id());

    if (!query.exec()) {
        kWarning() << query.lastError().text();
        return QList<QDate>();
    }

    QList<QDate> dates;
    while (query.next()) {
        dates << query.value(0).toDateTime().date();
    }

    return dates;
}

void PendingWinLoggerDates::queryFinished()
{
    QFutureWatcher<QList<QDate> > *watcher = static_cast<QFutureWatcher<QList<QDate> >* >(sender());
    const QList<QDate> dates = watcher->result();

    setDates(dates);
    watcher->deleteLater();

    emitFinished();
}

