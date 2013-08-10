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

#ifndef PENDINGWINLOGGERLOGS_H
#define PENDINGWINLOGGERLOGS_H

#include <KTp/Logger/pending-logger-logs.h>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

class PendingWinLoggerLogs : public KTp::PendingLoggerLogs
{
    Q_OBJECT

  public:
    explicit PendingWinLoggerLogs(const Tp::AccountPtr &account,
                                  const KTp::LogEntity &entity,
                                  const QDate &date,
                                  const QSqlDatabase &db,
                                  QObject* parent = 0);
    virtual ~PendingWinLoggerLogs();

  private Q_SLOTS:
    void queryFinished();

  private:
    QList<KTp::LogMessage> runQuery();
    KTp::LogEntity findSelfEntity(const QString &accountId,
                                  const Tp::AccountManagerPtr &accountManager) const;

    QSqlDatabase mDb;
};

#endif // PENDINGWINLOGGERLOGS_H
