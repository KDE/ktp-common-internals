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

class KTP_EXPORT LogManager : public AbstractLoggerPlugin
{
    Q_OBJECT

  public:
    static KTp::LogManager* instance();

    KTp::PendingLoggerDates* queryDates(const Tp::AccountPtr &account,
                                        const KTp::LogEntity &entity);
    KTp::PendingLoggerLogs*  queryLogs(const Tp::AccountPtr &account,
                                       const KTp::LogEntity &entity,
                                       const QDate &date);
    KTp::PendingLoggerEntities* queryEntities(const Tp::AccountPtr &account);

    virtual ~LogManager();

 private:
    explicit LogManager();

    class Private;
    Private * const d;

    friend class PendingLoggerOperation;

};

} // namespace KTp

#endif // KTP_LOGMANAGER_H
