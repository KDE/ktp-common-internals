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

#include "db.h"

#include <KStandardDirs>
#include <KDebug>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtCore/QDir>

Db* Db::s_instance = 0;

Db* Db::instance()
{
    if (s_instance == 0) {
        s_instance = new Db;
    }

    return s_instance;
}

Db::Db()
{
}

Db::~Db()
{
    closeDb();
}

QSqlDatabase Db::openDb()
{
    if (mDb.isOpen()) {
        return mDb;
    }

    QDir dir(KStandardDirs::locateLocal("data", QLatin1String("ktelepathy/logs/")));
    if (!dir.exists()) {
        dir.mkpath(dir.path());
    }

    mDb = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), QLatin1String("WinLoggerConnection"));
    mDb.setDatabaseName(dir.path() + QLatin1String("/winLogger.db"));
    if (!mDb.open()) {
        kWarning() << "Failed to open winLogger.db";
        mDb = QSqlDatabase();
    }

    return mDb;
}

void Db::closeDb()
{
    if (mDb.isOpen()) {
        mDb.close();
        QSqlDatabase::removeDatabase(QLatin1String("WinLoggerConnection"));
    }
}

bool Db::checkDb()
{
    QStringList tables = mDb.tables(QSql::AllTables);
    if (tables.isEmpty()) {
        initDb();
        return false;
    }

    return true;
}

void Db::initDb()
{
    QSqlQuery query(mDb);

  query.exec(QLatin1String("CREATE TABLE accounts ( "
                           "  id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                           "  uid TEXT NOT NULL)"));
  if (query.lastError().isValid()) {
    kWarning() << query.lastError().text();
    return;
  }

  query.exec(QLatin1String("CREATE TABLE contacts ( "
                           "  id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                           "  uid TEXT NOT NULL, "
                           "  name TEXT NOT NULL, "
                           "  accountId INTEGER, "
                           "  FOREIGN KEY(accountId) REFERENCES accounts(id))"));
  if (query.lastError().isValid()) {
    kWarning() << query.lastError().text();
    return;
  }

  query.exec(QLatin1String("CREATE TABLE logs ( "
                           "  id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                           "  datetime DATETIME, "
                           "  contactId INTEGER, "
                           "  message TEXT, "
                           "  FOREIGN KEY(contactId) REFERENCES contacts(id))"));
  if (query.lastError().isValid()) {
    kWarning() << query.lastError().text();
    return;
  }
}
