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

#ifndef KTP_ABSTRACTLOGGERPLUGIN_H
#define KTP_ABSTRACTLOGGERPLUGIN_H

#include <QtCore/QObject>

#include <KTp/ktp-export.h>

#include <TelepathyQt/Types>

namespace KTp {

class PendingLoggerDates;
class PendingLoggerLogs;
class PendingLoggerEntities;
class PendingLoggerSearch;
class LogEntity;

/**
 * @brief An interface for all KTp Logger plugins
 *
 * @since 0.7
 * @author Daniel Vrátil <dvratil@redhat.com>
 */
class KTP_EXPORT AbstractLoggerPlugin : public QObject
{
    Q_OBJECT

  public:
    /**
     * Constructor.
     */
    explicit AbstractLoggerPlugin(QObject *parent = 0);

    /**
     * Destructor.
     */
    virtual ~AbstractLoggerPlugin();

    /**
     * Queries all available plugins that handle given @p account for list of dates
     * with logs of user's chat with @p entity.
     *
     * @param account Account to query
     * @param entity Entity
     * @return Returns KTp::PendingLoggerDates operation that will emit finished()
     *         signal when all backends are finished.
     */
    virtual KTp::PendingLoggerDates* queryDates(const Tp::AccountPtr &account,
                                                const KTp::LogEntity &entity) = 0;

    /**
     * Queries all available plugins that handle given @p account for list of
     * logs of chats with @p entity.
     *
     * @param account Account to query
     * @param entity Entity whose logs should be retrieved
     * @param date Specific date for which to retrieve logs
     * @return Returns KTp::PendingLoggerLogs operation that will emit finished()
     *         signal when all backends are finished.
     */
    virtual KTp::PendingLoggerLogs* queryLogs(const Tp::AccountPtr &account,
                                              const KTp::LogEntity &entity,
                                              const QDate &date) = 0;

    /**
     * Queries all available plugins that handle given @p account for list of
     * entities for which they have conversation logs.
     *
     * @param account Account to query
     * @return Returns KTp::PendingLoggerEntities operation that will emit finished()
     *         signal when all backends are finished.
     */
    virtual KTp::PendingLoggerEntities* queryEntities(const Tp::AccountPtr &account) = 0;

    /**
     * Returnes whether plugin handles logs for given @p account.
     *
     * For example, a dedicated Facebook plugin will handle only accounts that
     * represent Facebook accounts, therefore it makes no sense to query it for
     * logs from GTalk account for instance.
     *
     * By default this method returns true, which means that plugin supports any
     * kind of account.
     */
    virtual bool handlesAccount(const Tp::AccountPtr &account);

    /**
     * Removes all logs for given @p account from all available plugins that
     * handle it.
     *
     * @param account Account of which to remove logs
     */
    virtual void clearAccountLogs(const Tp::AccountPtr &account) = 0;

    /**
     * Removes all logs for given @p entity from all available plugins that
     * handle the @p account.
     *
     * @param account Account to query
     * @param entity Entity whose logs to remove
     */
    virtual void clearContactLogs(const Tp::AccountPtr &account,
                                  const KTp::LogEntity &entity) = 0;

    /**
     * Searches all logs for given @p term.
     *
     * @param term Term to search
     * @return Returns KTp::PendingLoggerSearch operation that will emit finished()
     *         when all results are available
     */
    virtual KTp::PendingLoggerSearch* search(const QString &term) = 0;

    /**
     * Sets a new Tp::AccountManager to be used by the plugin.
     *
     * The @p accountManager is expected to be in ready state.
     *
     * @param accountManager An Tp::AccountManager in the ready state.
     */
    virtual void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    /**
     * Returns the set Tp::AccountManager or an empty pointer if none was set.
     */
    virtual Tp::AccountManagerPtr accountManager() const;

    /**
     * Checks whether there are any logs for given @p account and @p contact.
     *
     * For easy use this method is synchronous and can block for a while in case
     * of a slower plugin.
     *
     * @param account Account to query
     * @param contact Contact to query
     * @return Returns whether there are any logs for given person
     */
    virtual bool logsExist(const Tp::AccountPtr &account, const KTp::LogEntity &contact) = 0;

  private:
    class Private;
    Private * const d;
};

} // namespace KTp

#endif // KTP_ABSTRACTLOGGERPLUGIN_H
