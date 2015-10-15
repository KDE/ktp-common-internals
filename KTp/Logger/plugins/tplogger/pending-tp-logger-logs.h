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

#ifndef PENDINGTPLOGGERLOGS_H
#define PENDINGTPLOGGERLOGS_H

#include "KTp/Logger/pending-logger-logs.h"

namespace Tpl {
class PendingOperation;
}

class PendingTpLoggerLogs : public KTp::PendingLoggerLogs
{
    Q_OBJECT

  public:
    explicit PendingTpLoggerLogs(const Tp::AccountPtr &account,
                                 const KTp::LogEntity &entity,
                                 const QDate &date,
                                 QObject *parent = 0);
    virtual ~PendingTpLoggerLogs();

  private Q_SLOTS:
    void logsRetrieved(Tpl::PendingOperation *op);

};

#endif // PENDINGTPLOGGERLOGS_H
