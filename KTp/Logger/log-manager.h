/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef KTP_LOGMANAGER_H
#define KTP_LOGMANAGER_H

#include <KTp/Logger/abstract-logger-plugin.h>
#include <KTp/ktpcommoninternals_export.h>

#include <TelepathyQt/Types>

namespace KTp {

class LogEntity;

class PendingLoggerOperation;
class PendingLoggerDates;
class PendingLoggerLogs;
class PendingLoggerEntities;

/**
 * Log manager is a singleton that user can use to query the logs.
 *
 * @since 0.7
 * @author Daniel Vrátil <dvratil@redhat.com>
 */
class KTPCOMMONINTERNALS_EXPORT LogManager : public AbstractLoggerPlugin
{
    Q_OBJECT

  public:
    /**
     * Returns global instance of the LogManager.
     */
    static KTp::LogManager* instance();

    /**
     * Sets a new Tp::AccountManager to be used by the LogManager.
     *
     * The @p accountManager is expected to be in ready state.
     *
     * @param accountManager An Tp::AccountManager in the ready state.
     */
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    /**
     * Returns the set Tp::AccountManager or an empty pointer if none was set.
     */
    Tp::AccountManagerPtr accountManager() const;

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
     * handle it.
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
     * Searches all logs for given @p term.
     *
     * @param term Term to search
     * @return Returns KTp::PendingLoggerSearch operation that will emit finished()
     *         signal when search is finished.
     */
    KTp::PendingLoggerSearch* search(const QString &term);

    /**
     * Checks whether there are any logs for given @p account and @p contact.
     *
     * For easy use this method is synchronous and can block in case of a slower
     * plugin.
     *
     * @param account Account to query
     * @param contact Contact to query
     * @return Returns whether there are any logs for given person
     */
    bool logsExist(const Tp::AccountPtr &account, const KTp::LogEntity &contact);

    /**
     * Destructor.
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
