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

    // FIXME: Use SQLite's FTS feature for full-text-search
    bool prep = query.prepare(
                   QLatin1String("SELECT DISTINCT messages.datetime"
                                 "       messages.accountId, accounts.name"
                                 "       messages.contactId, contacts.uid, contacts.name, contacts.type"
                                 "       messages.direction "
                                 "FROM messages "
                                 "LEFT JOIN contacts AS contacts ON messages.contactId = contacts.id "
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
        const Tp::AccountPtr account = accounts.value(accountId);
        if (!account) {
            continue;
        }

        KTp::LogEntity entity;
        if (query.value(7).toInt() == 0) { // incomming message
            entity = KTp::LogEntity(static_cast<Tp::HandleType>(query.value(6).toInt()),
                                    query.value(4).toString(),
                                    query.value(5).toString());
        } else {
            const Tp::ContactPtr contact = account->connection()->selfContact();
            entity = KTp::LogEntity(Tp::HandleTypeContact,
                                    contact->id(), contact->alias());
        }

        hits << KTp::LogSearchHit(account, entity,
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



