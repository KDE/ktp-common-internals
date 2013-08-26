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

#ifndef TPLOGGERPLUGIN_H
#define TPLOGGERPLUGIN_H

#include "KTp/Logger/abstract-logger-plugin.h"

namespace Tpl {
class PendingOperation;
}

class TpLoggerPlugin : public KTp::AbstractLoggerPlugin
{
    Q_OBJECT

  public:
    explicit TpLoggerPlugin(QObject *parent, const QVariantList &);
    virtual ~TpLoggerPlugin();

    KTp::PendingLoggerDates* queryDates(const Tp::AccountPtr &account,
                                        const KTp::LogEntity &entity);
    KTp::PendingLoggerLogs* queryLogs(const Tp::AccountPtr &account,
                                      const KTp::LogEntity &entity,
                                      const QDate &date);
    KTp::PendingLoggerEntities* queryEntities(const Tp::AccountPtr &account);
    void clearAccountLogs(const Tp::AccountPtr &account);
    void clearContactLogs(const Tp::AccountPtr &account,
                          const KTp::LogEntity &entity);
    KTp::PendingLoggerSearch* search(const QString &term);
    bool logsExist(const Tp::AccountPtr &account, const KTp::LogEntity &contact);

    void setAccountManager(const Tp::AccountManagerPtr &accountManager);


  private Q_SLOTS:
    void genericOperationFinished(Tpl::PendingOperation *operation);
};

#endif // TPLOGGERPLUGIN_H
