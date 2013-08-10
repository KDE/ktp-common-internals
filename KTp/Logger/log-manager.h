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

#ifndef KTP_LOGMANAGER_H
#define KTP_LOGMANAGER_H

#include <KTp/Logger/abstract-logger-plugin.h>
#include <KTp/ktp-export.h>

#include <TelepathyQt/Types>

namespace KTp {

class LogEntity;

class PendingLoggerOperation;
class PendingLoggerDates;
class PendingLoggerLogs;
class PendingLoggerEntities;

/**
 * Log manager is a singleton that user can use to query the logs.
 */
class KTP_EXPORT LogManager : public AbstractLoggerPlugin
{
    Q_OBJECT

  public:
    /**
     * Returns global instance of the LogManager
     */
    static KTp::LogManager* instance();

    /**
     * Queries all available plugins for list of dates with logs of user's chat
     * with @p entity.
     *
     * @param account Account to query
     * @param entity Entity
     * @return Returns KTp::PendingLoggerDates operation that will emit finished()
     *         signal when all backends are finished.
     */
    KTp::PendingLoggerDates* queryDates(const Tp::AccountPtr &account,
                                        const KTp::LogEntity &entity);

    /**
     * Queries all available plugins for list of logs of chats with @p entity.
     *
     * @param account Account to query
     * @param entity Entity whose logs should be retrieved
     * @param date Specific date for which to retrieve logs
     * @return Returns KTp::PendingLoggerLogs operation that will emit finished()
     *         signal when all backends are finished.
     */
    KTp::PendingLoggerLogs*  queryLogs(const Tp::AccountPtr &account,
                                       const KTp::LogEntity &entity,
                                       const QDate &date);

   /**
     * Queries all available plugins for list of entities for which they have
     * conversation logs.
     *
     * @param account Account to query
     * @return Returns KTp::PendingLoggerEntities operation that will emit finished()
     *         signal when all backends are finished.
     */
    KTp::PendingLoggerEntities* queryEntities(const Tp::AccountPtr &account);

    /**
     * Removes all logs for given @p account from all available plugins that
     * handle it
     *
     * @param account Account of which to remove logs
     */
    void clearAccountLogs(const Tp::AccountPtr &account);

  /**
     * Removes all logs for given @p entity from all available plugins that
     * handle the @p account.
     *
     * @param account Account to query
     * @param entity Entity whose logs to remove
     */
    void clearContactLogs(const Tp::AccountPtr &account,
                          const KTp::LogEntity &entity);

    /**
     * Destructor
     */
    virtual ~LogManager();

 private:
    explicit LogManager();

    class Private;
    Private * const d;

    friend class PendingLoggerOperation;

};

} // namespace KTp

#endif // KTP_LOGMANAGER_H
