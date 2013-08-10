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

#include "pending-win-logger-search.h"
#include "log-search-hit.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtCore/QFuture>

#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/log-manager.h>

#include <KDebug>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/Account>

PendingWinLoggerSearch::PendingWinLoggerSearch(const QString &term,
                                               const QSqlDatabase &db,
                                               QObject *parent):
    PendingLoggerSearch(term, parent),
    mDb(db)
{
    QFuture<QList<KTp::LogSearchHit> > future = QtConcurrent::run(this, &PendingWinLoggerSearch::runQuery);
    QFutureWatcher<QList<KTp::LogSearchHit> > *watcher = new QFutureWatcher<QList<KTp::LogSearchHit> >(this);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()),
            this, SLOT(queryFinished()));
}

PendingWinLoggerSearch::~PendingWinLoggerSearch()
{
}

QList<KTp::LogSearchHit> PendingWinLoggerSearch::runQuery()
{
    QSqlQuery query(mDb);
    bool prep = query.prepare(
                   QLatin1String("SELECT DISTINCT messages.datetime"
                                 "       messages.accountId, accounts.name"
                                 "       messages.senderId, contacts1.uid, contacts1.name, contacts1.type"
                                 "       messages.receiverId, contacts2.uid, contacts2.name, contacts2.type"
                                 "       messages.direction "
                                 "FROM messages "
                                 "LEFT JOIN contacts AS contacts1 ON messages.senderId = contacts.id "
                                 "LEFT JOIN contacts AS contacts2 ON messages.receiverId = contacts.id "
                                 "WHERE messages.message LIKE '%:1%'"));
    if (!prep) {
        kWarning() << query.lastError().text();
        return QList<KTp::LogSearchHit>();
    }

    query.addBindValue(term());

    if (!query.exec()) {
        kWarning() << query.lastError().text();
        return QList<KTp::LogSearchHit>();
    }

    QList<KTp::LogSearchHit> hits;
    Tp::AccountManagerPtr accountManager = KTp::LogManager::instance()->accountManager();
    if (!accountManager || !accountManager->isValid()) {
        kWarning() << "No valid accountManager set";
        return hits;
    }

    QMap<QString, Tp::AccountPtr> accounts;
    Q_FOREACH (const Tp::AccountPtr &account, accountManager->validAccounts()->accounts()) {
        accounts.insert( account->uniqueIdentifier(), account);
    }

    while (query.next()) {
        const QString accountId = query.value(2).toString();
        if (!accounts.contains(accountId)) {
            continue;
        }

        KTp::LogEntity entity;
        if (query.value(11).toInt() == 1) { // outgoing message
            entity = KTp::LogEntity(static_cast<KTp::LogEntity::EntityType>(query.value(10).toInt()),
                                    query.value(8).toString(),
                                    query.value(9).toString());
        } else {
            entity = KTp::LogEntity(static_cast<KTp::LogEntity::EntityType>(query.value(6).toInt()),
                                    query.value(4).toString(),
                                    query.value(5).toString());
        }

        hits << KTp::LogSearchHit(accounts.value(accountId), entity,
                                  query.value(0).toDateTime().date());
    }

    return hits;
}

void PendingWinLoggerSearch::queryFinished()
{
    QFutureWatcher<QList<KTp::LogSearchHit> > *watcher = static_cast<QFutureWatcher<QList<KTp::LogSearchHit> >* >(sender());
    appendSearchHits(watcher->result());

    watcher->deleteLater();

    emitFinished();
}



