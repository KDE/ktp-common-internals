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
    kDebug() << "Found tables" << tables;
    if (!tables.contains(QLatin1String("accounts"))
            || !tables.contains(QLatin1String("contacts"))
            || !tables.contains(QLatin1String("messages")))
    {
        kDebug() << "Database does not exist or broken";
        return initDb();
    }

    return true;
}

bool Db::initDb()
{
    QSqlQuery query(mDb);
    query.exec(QLatin1String("DROP TABLE IF EXISTS accounts"));
    query.exec(QLatin1String("DROP TABLE IF EXISTS contacts"));
    query.exec(QLatin1String("DROP TABLE IF EXISTS messages"));

    query.exec(QLatin1String("CREATE TABLE accounts ( "
                            "  id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                            "  uid TEXT NOT NULL)"));
    if (query.lastError().isValid()) {
        kWarning() << query.lastError().text();
        return false;
    }
    kDebug() << "Created table 'accounts'";

    query.exec(QLatin1String("CREATE TABLE contacts ( "
                            "  id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                            "  uid TEXT NOT NULL, "
                            "  name TEXT NULL, "
                            "  type INTEGER NOT NULL)"));
    if (query.lastError().isValid()) {
        kWarning() << query.lastError().text();
        return false;
    }
    kDebug() << "Created table 'contacts'";

    query.exec(QLatin1String("CREATE TABLE messages ( "
                            "  id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                            "  direction INTEGER NOT NULL, "
                            "  datetime DATETIME NOT NULL, "
                            "  accountId INTEGER, "
                            "  contactId INTEGER, "
                            "  message TEXT NOT NULL, "
                            "  FOREIGN KEY(contactId) REFERENCES contacts(id),"
                            "  FOREIGN KEY(accountId) REFERENCES accounts(id))"));
    if (query.lastError().isValid()) {
        kWarning() << query.lastError().text();
        return false;
    }
    kDebug() << "Created table 'messages'";

    kDebug() << "Database successfully initialized";
    return true;
}

void Db::logMessage(const QString &accountId, const KTp::LogEntity &contact,
                    const Tp::Message &message, bool outgoing)
{
    int accId = getAccountId(accountId);
    if (accId == -1) {
        accId = storeAccount(accountId);
    }

    int contactId = getContactId(contact.id());
    if (contactId == -1) {
        contactId = storeContact(contact);
    }

    storeMessage(accId, outgoing, message.sent(), contactId, message.text());
}

int Db::getAccountId(const QString &accountUid)
{
    return getEntityId(QLatin1String("accounts"), accountUid);
}

int Db::getContactId(const QString &contactUid)
{
    return getEntityId(QLatin1String("contacts"), contactUid);
}

int Db::getEntityId(const QString &entityTable, const QString &entityUid)
{
    QSqlQuery query(mDb);
    if (!query.prepare(QString::fromLatin1("SELECT id FROM %1 WHERE uid=:1").arg(entityTable))) {
        handleError(query);
        return -1;
    }

    query.addBindValue(entityUid);

    if (!query.exec()) {
        handleError(query);
        return -1;
    }

    if (query.first()) {
        return query.value(0).toInt();
    }

    return -1;
}

int Db::storeAccount(const QString &accountId)
{
    QSqlQuery query(mDb);
    if (!query.prepare(QLatin1String("INSERT INTO accounts (uid) VALUES (:1)"))) {
        handleError(query);
        return -1;
    }

    query.addBindValue(accountId);

    if (!query.exec()) {
        handleError(query);
        return -1;
    }

    return query.lastInsertId().toInt();
}

int Db::storeContact(const KTp::LogEntity &contact)
{
    QSqlQuery query(mDb);
    if (!query.prepare(QLatin1String("INSERT INTO contacts (uid, name, type) VALUES(:1, :2, :3)"))) {
        handleError(query);
        return -1;
    }

    query.addBindValue(contact.id());
    query.addBindValue(contact.alias());
    query.addBindValue(static_cast<int>(contact.entityType()));

    if (!query.exec()) {
        handleError(query);
        return -1;
    }

    return query.lastInsertId().toInt();
}

int Db::storeMessage(int accountId, int messageType, const QDateTime &sent,
                     int contactId, const QString &messageText)
{
    QSqlQuery query(mDb);
    if (!query.prepare(QLatin1String("INSERT INTO messages (direction, datetime, accountId, contactId, message) "
                                     "VALUES(:1, :2, :3, :4, :5)"))) 
    {
        handleError(query);
        return -1;
    }

    query.addBindValue(messageType);
    query.addBindValue(sent.isValid() ? sent : QDateTime::currentDateTime());
    query.addBindValue(accountId);
    query.addBindValue(contactId);
    query.addBindValue(messageText);

    if (!query.exec()) {
        handleError(query);
        return -1;
    }

    return query.lastInsertId().toInt();
}

bool Db::removeAccount(int accountId)
{
    QSqlQuery query(mDb);
    if (!query.prepare(QLatin1String("DELETE FROM accounts WHERE id = :1"))) {
        handleError(query);
        return false;
    }

    query.addBindValue(accountId);

    if (!query.exec()) {
        handleError(query);
        return false;
    }

    return true;
}

bool Db::removeContact(int contactId)
{
    QSqlQuery query(mDb);
    if (!query.prepare(QLatin1String("DELETE FROM contacts WHERE id = :1"))) {
        handleError(query);
        return false;
    }

    query.addBindValue(contactId);

    if (!query.exec()) {
        handleError(query);
        return false;
    }

    return true;
}

void Db::removeAccountLogs(const QString& accountUid)
{
    const int accountId = getAccountId(accountUid);

    mDb.transaction();

    QSqlQuery query(mDb);
    if (!query.prepare(QLatin1String("DELETE FROM messages WHERE accountId = :1"))) {
        handleError(query);
        mDb.rollback();
        return;
    }

    query.addBindValue(accountId);

    if (!query.exec()) {
        handleError(query);
        mDb.rollback();
        return;
    }

    if (!removeAccount(accountId)) {
        mDb.rollback();
        return;
    }

    mDb.commit();
}

void Db::removeContactLogs(const QString& accountUid, const QString& contactUid)
{
    const int accountId = getAccountId(accountUid);
    const int contactId = getContactId(contactUid);

    mDb.transaction();

    QSqlQuery query(mDb);
    if (!query.prepare(QLatin1String("DELETE FROM messages "
                                     "WHERE accountId = :1 AND contactId = :2"))) {
        handleError(query);
        mDb.rollback();
        return;
    }

    query.addBindValue(accountId);
    query.addBindValue(contactId);

    if (!query.exec()) {
        handleError(query);
        mDb.rollback();
        return;
    }

    if (!removeContact(contactId)) {
        mDb.rollback();
        return;
    }

    if (!removeAccount(accountId)) {
        mDb.rollback();
        return;
    }

    mDb.commit();
}

void Db::handleError(const QSqlQuery &query)
{
    kWarning() << "SQL ERROR:" << query.lastError().text();
    kWarning() << "Query was:" << query.executedQuery();
}
