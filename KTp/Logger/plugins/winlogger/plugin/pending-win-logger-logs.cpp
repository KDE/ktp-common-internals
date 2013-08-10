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

#include "pending-win-logger-logs.h"

#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/log-message.h>

#include <KDebug>
#include <KTp/Logger/log-manager.h>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <TelepathyQt/AccountSet>

PendingWinLoggerLogs::PendingWinLoggerLogs(const Tp::AccountPtr &account,
                                           const KTp::LogEntity &entity,
                                           const QDate &date,
                                           const QSqlDatabase &db,
                                           QObject *parent): 
    PendingLoggerLogs(account, entity, date, parent),
    mDb(db)
{
    QFuture<QList<KTp::LogMessage> > future = QtConcurrent::run(this, &PendingWinLoggerLogs::runQuery);
    QFutureWatcher<QList<KTp::LogMessage> > *watcher = new QFutureWatcher<QList<KTp::LogMessage> >(this);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()),
            this, SLOT(queryFinished()));
}

PendingWinLoggerLogs::~PendingWinLoggerLogs()
{
}

QList<KTp::LogMessage> PendingWinLoggerLogs::runQuery()
{
    QSqlQuery query(mDb);
    if (!query.prepare(QLatin1String("SELECT messages.datetime, messages.message, messages.direction, "
                                     "       contacts.type, contacts.uid, contacts.name, "
                                     "       accounts.uid "
                                     "FROM messages "
                                     "LEFT JOIN contacts ON messages.contactId = contacts.id "
                                     "LEFT JOIN accounts ON messages.accountId = accounts.id "
                                     "WHERE accounts.uid = :1 AND contacts.uid = :2 "
                                     "AND messages.datetime >= :3 AND messages.datetime <= :4"))) {
        kWarning() << query.lastError().text();
        return QList<KTp::LogMessage>();
    }

    query.addBindValue(account()->uniqueIdentifier());
    query.addBindValue(entity().id());
    query.addBindValue(QDateTime(date()));
    query.addBindValue(QDateTime(date(), QTime(23, 59, 59)));

    if (!query.exec()) {
        kWarning() << query.lastError().text();
        return QList<KTp::LogMessage>();
    }

    Tp::AccountManagerPtr accountManager = KTp::LogManager::instance()->accountManager();
    if (!accountManager || !accountManager->isValid()) {
        kWarning() << "No valid accountManager set";
        return QList<KTp::LogMessage>();
    }

    QList<KTp::LogMessage> messages;
    KTp::LogEntity selfContact;
    while (query.next()) {
        KTp::LogEntity entity;

        if (query.value(2).toInt() == 0)  { // incomming message
            entity = KTp::LogEntity(static_cast<KTp::LogEntity::EntityType>(query.value(3).toInt()),
                                    query.value(4).toString(),
                                    query.value(5).toString());
        } else {
            if (!selfContact.isValid()) {
                selfContact = findSelfEntity(query.value(6).toString(), accountManager);
                if (!selfContact.isValid()) {
                    kWarning() << "Failed to retrieve self contact, bailing";
                    return QList<KTp::LogMessage>();
                }
            }

            entity = selfContact;
        }

        KTp::LogMessage message(entity, account(), query.value(0).toDateTime(),
                             query.value(1).toString());

        messages << message;
    }

    return messages;
}

void PendingWinLoggerLogs::queryFinished()
{
    QFutureWatcher<QList<KTp::LogMessage> > *watcher = static_cast<QFutureWatcher<QList<KTp::LogMessage> >* >(sender());
    const QList<KTp::LogMessage> messages = watcher->result();

    appendLogs(messages);
    watcher->deleteLater();

    emitFinished();
}

KTp::LogEntity PendingWinLoggerLogs::findSelfEntity(const QString &accountId,
                                                    const Tp::AccountManagerPtr &accountManager) const
{
    const QList<Tp::AccountPtr> accounts = accountManager->validAccounts()->accounts();
    Tp::AccountPtr logAccount;
    Q_FOREACH (const Tp::AccountPtr &account, accounts) {
        if (account->uniqueIdentifier() == accountId) {
            logAccount = account;
            break;
        }
    }

    if (!logAccount) {
        kWarning() << "Failed to find valid account" << accountId;
        return KTp::LogEntity();
    }

    const Tp::ContactPtr contact = logAccount->connection()->selfContact();
    if (!contact) {
        kWarning() << "Failed to retrieve selfContact";
        return KTp::LogEntity();
    }

    return KTp::LogEntity(KTp::LogEntity::EntityTypeContact, contact->id(),
                          contact->alias());
}
