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

#include "pending-logger-dates-impl.h"
#include "pending-logger-logs-impl.h"

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

PendingLoggerDates* LogManager::queryDates(const Tp::AccountPtr &account,
                                           const Tp::ContactPtr &contact)
{
    return new PendingLoggerDatesImpl(account, contact, this);
}

PendingLoggerLogs* LogManager::queryLogs(const Tp::AccountPtr &account,
                                         const Tp::ContactPtr &contact,
                                         const QDate &date)
{
    return new PendingLoggerLogsImpl(account, contact, date, this);
}

using namespace KTp;
