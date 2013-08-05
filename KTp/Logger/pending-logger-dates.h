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

#ifndef KTP_PENDINGLOGGERDATES_H
#define KTP_PENDINGLOGGERDATES_H

#include <KTp/Logger/pending-logger-operation.h>
#include <TelepathyQt/Types>

namespace KTp {

class PendingLoggerDates : public KTp::PendingLoggerOperation
{
    Q_OBJECT

  public:
    virtual ~PendingLoggerDates();

    Tp::AccountPtr account() const;
    Tp::ContactPtr contact() const;
    QList<QDate> dates() const;

  protected:
    explicit PendingLoggerDates(const Tp::AccountPtr &account,
                                const Tp::ContactPtr &contact,
                                QObject *parent = 0);

    void setDates(const QList<QDate> &dates);

    class Private;
    Private * const d;
};
}

#endif // KTP_PENDINGLOGGERDATES_H
