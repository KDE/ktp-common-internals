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

#ifndef KTP_PENDINGLOGGERLOGS_H
#define KTP_PENDINGLOGGERLOGS_H

#include <KTp/Logger/pending-logger-operation.h>
#include <KTp/Logger/log-message.h>
#include <KTp/ktpcommoninternals_export.h>

#include <TelepathyQt/Types>

namespace KTp {

/**
 * @brief An operation that will retrieve list of chat logs for given account,
 *        entity and date from all backends.
 *
 * The operation will emit finished(KTp::PendingLoggerOperation*) signal when
 * all logs were retrieved. When an error occurs in any backend hasError()
 * will be set to true. Use error() to retrieve the error message.
 *
 * @since 0.7
 * @author Daniel Vrátil <dvratil@redhat.com>
 */
class KTPCOMMONINTERNALS_EXPORT PendingLoggerLogs : public KTp::PendingLoggerOperation
{
    Q_OBJECT

  public:
    /**
     * Destructor.
     */
    virtual ~PendingLoggerLogs();

    /**
     * Returns account for which logs are being queried.
     */
    Tp::AccountPtr account() const;

    /**
     * Returns entity for which logs are being queried.
     */
    KTp::LogEntity entity() const;

    /**
     * Returns date for which logs are being queried.
     */
    QDate date() const;

    /**
     * Returns list of retrieved log messages.
     */
    QList<KTp::LogMessage> logs() const;


  protected:
    explicit PendingLoggerLogs(const Tp::AccountPtr &account,
                               const KTp::LogEntity &entity,
                               const QDate &date,
                               QObject* parent = 0);

    void appendLogs(const QList<KTp::LogMessage> &logs);

    class Private;
    Private * const d;
};

} // namespace KTp

#endif // KTP_PENDINGLOGGERLOGS_H
