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

#ifndef KTP_PENDINGLOGGERSEARCH_H
#define KTP_PENDINGLOGGERSEARCH_H

#include <KTp/Logger/pending-logger-operation.h>
#include <KTp/Logger/log-search-hit.h>

#include <KTp/ktpcommoninternals_export.h>

namespace KTp {

/**
 * @brief An operation that will retrieve list of chat logs matching given search
 *        term.
 *
 * The operation will emit finished(KTp::PendingLoggerOperation*) signal when
 * search is finished. When an error occurs in any backend hasError() will be
 * set to true. Use error() to retrieve the error message.
 *
 * @since 0.7
 * @author Daniel Vrátil <dvratil@redhat.com>
 */
class KTPCOMMONINTERNALS_EXPORT PendingLoggerSearch : public  KTp::PendingLoggerOperation
{
    Q_OBJECT

  public:
    /**
     * Destructor.
     */
    virtual ~PendingLoggerSearch();

    /**
     * Returns the search term that is used.
     */
    QString term() const;

    /**
     * Returns list of hits matching givem search term.
     */
    QList<KTp::LogSearchHit> searchHits() const;

  protected:
    explicit PendingLoggerSearch(const QString &term,
                                 QObject *parent = 0);

    void appendSearchHits(const QList<KTp::LogSearchHit> &searchHits);
    void appendSearchHit(const KTp::LogSearchHit &searchHit);

  private:
    class Private;
    Private * const d;

};

} // namespace KTp

#endif // KTP_PENDINGLOGGERSEARCH_H
