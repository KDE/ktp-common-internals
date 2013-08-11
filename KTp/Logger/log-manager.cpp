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

#include "log-manager.h"
#include "log-manager-private.h"
#include "abstract-logger-plugin.h"
#include "log-entity.h"

#include "pending-logger-dates-impl.h"
#include "pending-logger-logs-impl.h"
#include "pending-logger-entities-impl.h"
#include "pending-logger-search-impl.h"

#include <KGlobal>
#include <KService>
#include <KServiceTypeTrader>
#include <KPluginInfo>

#include <KDebug>

using namespace KTp;

#define KTP_LOGGER_PLUGIN_VERSION "1"

LogManager* LogManager::Private::s_logManagerInstance = 0;

void LogManager::Private::loadPlugins()
{
    const KService::List services = KServiceTypeTrader::self()->query(
                                         QLatin1String("KTpLogger/Plugin"),
                                         QLatin1String("[X-KTp-PluginInfo-Version] == " KTP_LOGGER_PLUGIN_VERSION));

    const KPluginInfo::List pluginInfos = KPluginInfo::fromServices(services);
    Q_FOREACH (const KPluginInfo &pluginInfo, pluginInfos) {
        const KService::Ptr service = pluginInfo.service();
        KPluginFactory *factory = KPluginLoader(service->library()).factory();
        if (factory) {
            kDebug() << "loaded factory :" << factory;
            AbstractLoggerPlugin *plugin = factory->create<AbstractLoggerPlugin>(q);

            if (plugin) {
                kDebug() << "loaded logger plugin : " << plugin;
                plugins << plugin;
            }
        } else {
            kError() << "error loading plugin :" << service->library();
        }
    }
}

LogManager::Private::Private(LogManager *parent):
    q(parent)
{
    loadPlugins();
}

LogManager* LogManager::instance()
{
    if (Private::s_logManagerInstance == 0) {
        Private::s_logManagerInstance = new LogManager();
    }

    return Private::s_logManagerInstance;
}

LogManager::LogManager():
    AbstractLoggerPlugin(),
    d(new Private(this))
{
}

LogManager::~LogManager()
{
    delete d;
}

Tp::AccountManagerPtr LogManager::accountManager() const
{
    if (d->plugins.isEmpty()) {
        return Tp::AccountManagerPtr();
    }

    return d->plugins.first()->accountManager();
}

void LogManager::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    Q_FOREACH (KTp::AbstractLoggerPlugin *plugin, d->plugins) {
        plugin->setAccountManager(accountManager);
    }
}


PendingLoggerDates* LogManager::queryDates(const Tp::AccountPtr &account,
                                           const KTp::LogEntity &entity)
{
    return new PendingLoggerDatesImpl(account, entity, this);
}

PendingLoggerLogs* LogManager::queryLogs(const Tp::AccountPtr &account,
                                         const KTp::LogEntity &entity,
                                         const QDate &date)
{
    return new PendingLoggerLogsImpl(account, entity, date, this);
}

PendingLoggerEntities* LogManager::queryEntities(const Tp::AccountPtr& account)
{
    return new PendingLoggerEntitiesImpl(account, this);
}

void LogManager::clearAccountLogs(const Tp::AccountPtr &account)
{
   Q_FOREACH (KTp::AbstractLoggerPlugin *plugin, d->plugins) {
        if (!plugin->handlesAccount(account)) {
            continue;
        }

        plugin->clearAccountLogs(account);
    }
}

void LogManager::clearContactLogs(const Tp::AccountPtr &account,
                                  const KTp::LogEntity &entity)
{
    Q_FOREACH (KTp::AbstractLoggerPlugin *plugin, d->plugins) {
        if (!plugin->handlesAccount(account)) {
            continue;
        }

        plugin->clearContactLogs(account, entity);
    }
}

PendingLoggerSearch* LogManager::search(const QString& term)
{
    return new PendingLoggerSearchImpl(term, this);
}

bool LogManager::logsExist(const Tp::AccountPtr &account, const KTp::LogEntity &contact)
{
    Q_FOREACH (KTp::AbstractLoggerPlugin *plugin, d->plugins) {
        if (!plugin->handlesAccount(account)) {
            continue;
        }

        if (plugin->logsExist(account, contact)) {
            return true;
        }
    }

    return false;
}


using namespace KTp;
