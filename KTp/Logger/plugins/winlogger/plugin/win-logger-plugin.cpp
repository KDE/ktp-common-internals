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

#include "win-logger-plugin.h"
#include "../logger/db.h"

#include "pending-win-logger-dates.h"
#include "pending-win-logger-entities.h"
#include "pending-win-logger-logs.h"
#include "pending-win-logger-search.h"

#include <KPluginFactory>
#include <QSqlError>

WinLogger::WinLogger(QObject *parent, const QVariantList &):
    AbstractLoggerPlugin(parent)
{
    mDb = Db::instance()->openDb();
    if (!mDb.isValid()) {
        return;
    }

    Db::instance()->checkDb();
}

WinLogger::~WinLogger()
{
    Db::instance()->closeDb();
}

KTp::PendingLoggerDates* WinLogger::queryDates(const Tp::AccountPtr &account,
                                               const KTp::LogEntity &entity)
{
    if (!mDb.isOpen()) {
        return 0;
    }

    return new PendingWinLoggerDates(account, entity, mDb, this);
}

KTp::PendingLoggerEntities* WinLogger::queryEntities(const Tp::AccountPtr &account)
{
    if (!mDb.isOpen()) {
        return 0;
    }

    return new PendingWinLoggerEntities(account, mDb, this);
}

KTp::PendingLoggerLogs* WinLogger::queryLogs(const Tp::AccountPtr &account,
                                             const KTp::LogEntity &entity,
                                             const QDate &date)
{
    if (!mDb.isOpen()) {
        return 0;
    }

    return new PendingWinLoggerLogs(account, entity, date, mDb, this);
}

void WinLogger::clearAccountLogs(const Tp::AccountPtr &account)
{
    if (!mDb.isOpen()) {
        return;
    }

    Db::instance()->removeAccountLogs(account->uniqueIdentifier());
}

void WinLogger::clearContactLogs(const Tp::AccountPtr &account,
                                 const KTp::LogEntity &entity)
{
    if (!mDb.isOpen()) {
        return;
    }

    Db::instance()->removeContactLogs(account->uniqueIdentifier(), entity.id());
}

KTp::PendingLoggerSearch* WinLogger::search(const QString &term)
{
    if (!mDb.isOpen()) {
        return 0;
    }

    return new PendingWinLoggerSearch(term, mDb, this);
}

bool WinLogger::logsExist(const Tp::AccountPtr &account, const KTp::LogEntity &contact)
{
    if (!mDb.isOpen()) {
        return false;
    }

    const int accountId = Db::instance()->getAccountId(account->uniqueIdentifier());
    if (accountId == -1) {
        return false;
    }

    const int contactId = Db::instance()->getContactId(contact.id());
    if (contactId == -1) {
        return false;
    }

    QSqlQuery query(mDb);
    if (!query.prepare(QLatin1String("SELECT COUNT(id) FROM messages "
                                     "WHERE accountId = :1 AND contactId = :2")))
    {
        return false;
    }

    query.addBindValue(accountId);
    query.addBindValue(contactId);

    if (!query.exec() || !query.first()) {
        return false;
    }

    const int results = query.value(0).toInt();
    return (results > 0);
}


K_PLUGIN_FACTORY(WinLoggerFactory, registerPlugin<WinLogger>();)
K_EXPORT_PLUGIN(WinLoggerFactory("ktp_logger_plugin_winlogger"))
