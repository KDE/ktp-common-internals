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

#include "tp-logger-plugin.h"
#include "pending-tp-logger-dates.h"
#include "pending-tp-logger-entities.h"
#include "pending-tp-logger-logs.h"
#include "utils.h"
#include "pending-tp-logger-search.h"

#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/Init>
#include <TelepathyLoggerQt4/PendingOperation>

#include <KDebug>
#include <KPluginFactory>

TpLoggerPlugin::TpLoggerPlugin(QObject *parent, const QVariantList &):
    AbstractLoggerPlugin(parent)
{
    Tpl::init();
}

TpLoggerPlugin::~TpLoggerPlugin()
{
}

void TpLoggerPlugin::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    Tpl::LogManager::instance()->setAccountManagerPtr(accountManager);
    AbstractLoggerPlugin::setAccountManager(accountManager);
}

KTp::PendingLoggerDates* TpLoggerPlugin::queryDates(const Tp::AccountPtr &account,
                                                    const KTp::LogEntity &entity)
{
    return new PendingTpLoggerDates(account, entity, this);
}

KTp::PendingLoggerLogs* TpLoggerPlugin::queryLogs(const Tp::AccountPtr &account,
                                                  const KTp::LogEntity &entity,
                                                  const QDate &date)
{
    return new PendingTpLoggerLogs(account, entity, date, this);
}

KTp::PendingLoggerEntities* TpLoggerPlugin::queryEntities(const Tp::AccountPtr& account)
{
    return new PendingTpLoggerEntities(account, this);
}

void TpLoggerPlugin::clearAccountLogs(const Tp::AccountPtr &account)
{
    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tpl::PendingOperation *op = manager->clearAccountHistory(account);
    connect(op, SIGNAL(finished(Tpl::PendingOperation*)),
            this, SLOT(genericOperationFinished(Tpl::PendingOperation*)));
}

void TpLoggerPlugin::clearContactLogs(const Tp::AccountPtr &account,
                                      const KTp::LogEntity &entity)
{
    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tpl::PendingOperation *op = manager->clearEntityHistory(account, Utils::toTplEntity(entity));
    connect(op, SIGNAL(finished(Tpl::PendingOperation*)),
            this, SLOT(genericOperationFinished(Tpl::PendingOperation*)));
}

void TpLoggerPlugin::genericOperationFinished(Tpl::PendingOperation *operation)
{
    if (operation->isError()) {
        kWarning() << operation->errorName() << ":" << operation->errorMessage();
    }
}

KTp::PendingLoggerSearch* TpLoggerPlugin::search(const QString &term)
{
    return new PendingTpLoggerSearch(term, this);
}

bool TpLoggerPlugin::logsExist(const Tp::AccountPtr &account, const KTp::LogEntity &contact)
{
    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    return manager->exists(account, Utils::toTplEntity(contact), Tpl::EventTypeMaskText);
}


K_PLUGIN_FACTORY(TpLoggerPluginFactory, registerPlugin<TpLoggerPlugin>();)
K_EXPORT_PLUGIN(TpLoggerPluginFactory("ktp_logger_plugin_tpLogger"))
