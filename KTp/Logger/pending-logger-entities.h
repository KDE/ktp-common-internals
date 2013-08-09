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

#ifndef KTP_PENDINGLOGGERENTITIES_H
#define KTP_PENDINGLOGGERENTITIES_H

#include <KTp/Logger/pending-logger-operation.h>
#include <KTp/Logger/log-entity.h>
#include <KTp/ktp-export.h>

#include <TelepathyQt/Types>

namespace KTp {

class KTP_EXPORT PendingLoggerEntities : public KTp::PendingLoggerOperation
{
    Q_OBJECT

  public:
    virtual ~PendingLoggerEntities();

    Tp::AccountPtr account() const;
    QList<KTp::LogEntity> entities() const;

  protected:
    explicit PendingLoggerEntities(const Tp::AccountPtr &account, QObject* parent = 0);

    void appendEntities(const QList<KTp::LogEntity> &entities);
    void appendEntity(const KTp::LogEntity &entity);

    class Private;
    Private * const d;
};

}

#endif // KTP_PENDINGLOGGERENTITIES_H
