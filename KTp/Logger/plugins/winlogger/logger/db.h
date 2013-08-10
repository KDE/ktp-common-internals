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

#ifndef DB_H
#define DB_H

#include <QtSql/QSqlDatabase>

#include <KTp/Logger/log-entity.h>

#include <TelepathyQt/Message>

class QSqlQuery;

class Db
{
  public:
    static Db* instance();
    ~Db();

    QSqlDatabase openDb();
    void closeDb();

    bool checkDb();

    void logMessage(const QString &accountId,
                    const KTp::LogEntity &contact,
                    const Tp::Message &message,
                    bool outgoing);

    int getAccountId(const QString &accountUid);
    int storeAccount(const QString &accountUid);
    int getContactId(const QString &contactUid);
    int storeContact(const KTp::LogEntity &contact);
    bool removeAccount(int accountId);
    bool removeContact(int contactId);

    int storeMessage(int accountId, int direction, const QDateTime &sent,
                     int contactId, const QString &messageText);

    void removeAccountLogs(const QString &accountUid);
    void removeContactLogs(const QString &accountUid, const QString &contactUid);

  private:
    static Db *s_instance;
    Db();

    bool initDb();
    void handleError(const QSqlQuery &query);

    int getEntityId(const QString &entityTable, const QString &entityUid);

    QSqlDatabase mDb;
};

#endif // DB_H
