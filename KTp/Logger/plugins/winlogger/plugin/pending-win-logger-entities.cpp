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

#include "pending-win-logger-entities.h"

#include <KTp/Logger/log-entity.h>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <KDebug>

#include <TelepathyQt/Account>

PendingWinLoggerEntities::PendingWinLoggerEntities(const Tp::AccountPtr &account,
                                                   const QSqlDatabase &db,
                                                   QObject* parent):
    PendingLoggerEntities(account, parent),
    mDb(db)
{
    QFuture<QList<KTp::LogEntity> > future = QtConcurrent::run(this, &PendingWinLoggerEntities::runQuery);
    QFutureWatcher<QList<KTp::LogEntity> > *watcher = new QFutureWatcher<QList<KTp::LogEntity> >(this);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()),
            this, SLOT(queryFinished()));
}

PendingWinLoggerEntities::~PendingWinLoggerEntities()
{
}

QList<KTp::LogEntity> PendingWinLoggerEntities::runQuery()
{
    QSqlQuery query(mDb);
    if (!query.prepare(QLatin1String("SELECT contacts.type, contacts.uid, contacts.name FROM contacts "
                                     "LEFT JOIN accounts ON contacts.accountId = accounts.id "
                                     "WHERE accounts.uid = :1"))) {
        kWarning() << query.lastError().text();
        return QList<KTp::LogEntity>();
    }

    query.addBindValue(account()->uniqueIdentifier());

    if (!query.exec()) {
        kWarning() << query.lastError().text();
        return QList<KTp::LogEntity>();
    }

    QList<KTp::LogEntity> entities;
    while (query.next()) {
        entities << KTp::LogEntity(static_cast<KTp::LogEntity::EntityType>(query.value(0).toInt()),
                                   query.value(1).toString(),
                                   query.value(2).toString());
    }

    return entities;
}

void PendingWinLoggerEntities::queryFinished()
{
    QFutureWatcher<QList<KTp::LogEntity> > *watcher = static_cast<QFutureWatcher<QList<KTp::LogEntity> >* >(sender());
    const QList<KTp::LogEntity> entities = watcher->result();

    appendEntities(entities);
    watcher->deleteLater();

    emitFinished();
}
