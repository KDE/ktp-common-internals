/*
 *    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "message-filter-config-manager.h"
#include "message-processor-private.h"
#include "version.h"

#include <QMutex>
#include <QSet>

#include <KGlobal>
#include <KDebug>
#include <KServiceTypeTrader>

typedef QSet<KPluginInfo> PluginSet;

using namespace KTp;

class MessageFilterConfigManager::Private
{
  public:
    Private(MessageFilterConfigManager *parent):
        q(parent)
    { }

    PluginSet all;
    PluginSet enabled;

    KService::List offers() const;
    void generateCache();

  private:
    MessageFilterConfigManager *q;
};

KService::List MessageFilterConfigManager::Private::offers() const
{
    return KServiceTypeTrader::self()->query(QLatin1String("KTpTextUi/MessageFilter"),
                                             QLatin1String("[X-KTp-PluginInfo-Version] == " KTP_MESSAGE_FILTER_FRAMEWORK_VERSION));
}


void MessageFilterConfigManager::Private::generateCache()
{
    KPluginInfo::List pluginInfos = KPluginInfo::fromServices(offers(), q->configGroup());
    for (KPluginInfo::List::Iterator i = pluginInfos.begin(); i != pluginInfos.end(); i++) {
        KPluginInfo &plugin = *i;

        all.insert(plugin);

        plugin.load();
        if (plugin.isPluginEnabled()) {
            enabled.insert(plugin);
        }
    }
}

MessageFilterConfigManager *MessageFilterConfigManager::self()
{
    static MessageFilterConfigManager *mfcm_instance;
    static QMutex mutex;
    mutex.lock();
    if (!mfcm_instance) {
        mfcm_instance = new MessageFilterConfigManager;
    }
    mutex.unlock();

    return mfcm_instance;
}

MessageFilterConfigManager::MessageFilterConfigManager() :
    d(new Private(this))
{
    d->generateCache();
}

MessageFilterConfigManager::~MessageFilterConfigManager()
{
    delete d;
}

KPluginInfo::List MessageFilterConfigManager::allPlugins() const
{
    return d->all.toList();
}

KPluginInfo::List MessageFilterConfigManager::enabledPlugins() const
{
    return d->enabled.toList();
}

KConfigGroup MessageFilterConfigManager::configGroup() const
{
    return sharedConfig()->group("Plugins");
}

KSharedConfig::Ptr MessageFilterConfigManager::sharedConfig() const
{
    return KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
}

void MessageFilterConfigManager::reloadConfig()
{
    PluginSet::ConstIterator iter = d->all.constBegin();
    for ( ; iter != d->all.constEnd(); ++iter) {
        KPluginInfo pluginInfo = *iter;

        const bool wasEnabled = d->enabled.contains(pluginInfo);

        if (!wasEnabled && pluginInfo.isPluginEnabled()) {
            d->enabled.insert(pluginInfo);
            MessageProcessor::instance()->d->loadFilter(pluginInfo);
        } else if (wasEnabled && !pluginInfo.isPluginEnabled()) {
            d->enabled.remove(pluginInfo);
            MessageProcessor::instance()->d->unloadFilter(pluginInfo);
        }
    }
}
