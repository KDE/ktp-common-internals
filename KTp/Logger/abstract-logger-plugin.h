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
class LogEntity;

/**
 * @brief An interface for all KTp Logger plugins
 */
class KTP_EXPORT AbstractLoggerPlugin : public QObject
{
    Q_OBJECT

  public:
    explicit AbstractLoggerPlugin(QObject *parent = 0);
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

};

}

#endif // KTP_ABSTRACTLOGGERPLUGIN_H
