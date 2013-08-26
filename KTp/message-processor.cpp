/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "message-processor.h"
#include "message-processor-private.h"
#include "message-filters-private.h"
#include "message-filter-config-manager.h"

#include <QMutex>
#include <QStringBuilder>

#include <KDebug>
#include <KService>
#include <KServiceTypeTrader>
#include <KPluginFactory>
#include <KDE/KStandardDirs>

using namespace KTp;

FilterPlugin::FilterPlugin(const KPluginInfo &pluginInfo, KTp::AbstractMessageFilter *instance_):
    name(pluginInfo.pluginName()),
    instance(instance_)
{
    bool ok;
    weight = pluginInfo.service()->property(QLatin1String("X-KDE-PluginInfo-Weight"), QVariant::Int).toInt(&ok);
    if (!ok) {
        weight = 100;
    }
}

FilterPlugin::FilterPlugin(const QString &name_, int weight_, KTp::AbstractMessageFilter *instance_):
    name(name_),
    weight(weight_),
    instance(instance_)
{
}

bool FilterPlugin::operator<(const FilterPlugin &other) const
{
    return weight < other.weight;
}

bool FilterPlugin::operator==(const FilterPlugin &other) const
{
    return instance == other.instance &&
           name == other.name &&
           weight == other.weight;
}

void MessageProcessor::Private::loadFilter(const KPluginInfo &pluginInfo)
{
    KService::Ptr service = pluginInfo.service();

    KPluginFactory *factory = KPluginLoader(service->library()).factory();
    if (factory) {
        kDebug() << "loaded factory :" << factory;
        AbstractMessageFilter *filter = factory->create<AbstractMessageFilter>(q);

        if (filter) {
            kDebug() << "loaded message filter : " << filter;
            filters << FilterPlugin(pluginInfo, filter);
        }
    } else {
        kError() << "error loading plugin :" << service->library();
    }

    // Re-sort filters by weight
    qSort(filters);
}

void MessageProcessor::Private::unloadFilter(const KPluginInfo &pluginInfo)
{
    QList<FilterPlugin>::Iterator iter = filters.begin();
    for ( ; iter != filters.end(); ++iter) {
        const FilterPlugin &plugin = *iter;

        if (plugin.name == pluginInfo.pluginName()) {
            kDebug() << "unloading message filter : " << plugin.instance;
            plugin.instance->deleteLater();
            filters.erase(iter);
            return;
        }
    }
}

void MessageProcessor::Private::loadFilters()
{
    kDebug() << "Starting loading filters...";

    KPluginInfo::List plugins = MessageFilterConfigManager::self()->enabledPlugins();

    Q_FOREACH (const KPluginInfo &plugin, plugins) {
        loadFilter(plugin);
    }
}

KTp::MessageProcessor* MessageProcessor::instance()
{
    kDebug();

    static KTp::MessageProcessor *mp_instance;
    static QMutex mutex;
    mutex.lock();
    if (!mp_instance) {
        mp_instance= new MessageProcessor;
    }
    mutex.unlock();

    return mp_instance;
}


MessageProcessor::MessageProcessor():
    d(new MessageProcessor::Private(this))
{
    // Default weight is 100. Make sure these two plugins are always above those
    // which don't have weight specified and in this exact order.
    d->filters << FilterPlugin(QLatin1String("__messageEscapeFilter"), 98, new MessageEscapeFilter(this));
    d->filters << FilterPlugin(QLatin1String("__messageUrlFilter"), 99, new MessageUrlFilter(this));

    d->loadFilters();
}


MessageProcessor::~MessageProcessor()
{
    delete d;
}

QString MessageProcessor::header()
{
    QStringList scripts;
    QStringList stylesheets;
    Q_FOREACH (const FilterPlugin &plugin, d->filters) {
        Q_FOREACH (const QString &script, plugin.instance->requiredScripts()) {
            // Avoid duplicates
            if (!scripts.contains(script)) {
                scripts << script;
            }
        }
        Q_FOREACH (const QString &stylesheet, plugin.instance->requiredStylesheets()) {
            // Avoid duplicates
            if (!stylesheets.contains(stylesheet)) {
                stylesheets << stylesheet;
            }
        }
    }

    QString out(QLatin1String("\n    <!-- The following scripts and stylesheets are injected here by the plugins -->\n"));
    Q_FOREACH(const QString &script, scripts) {
        out = out % QLatin1String("    <script type=\"text/javascript\" src=\"")
                  % KGlobal::dirs()->findResource("data", script)
                  % QLatin1String("\"></script>\n");
    }
    Q_FOREACH(const QString &stylesheet, stylesheets) {
        out = out % QLatin1String("    <link rel=\"stylesheet\" type=\"text/css\" href=\"")
                  % KGlobal::dirs()->findResource("data", stylesheet)
                  % QLatin1String("\" />\n");
    }

    kDebug() << out;

    return out;
}

KTp::Message MessageProcessor::processIncomingMessage(const Tp::Message &message, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel)
{
    KTp::MessageContext context(account, channel);
    return processIncomingMessage(KTp::Message(message, context), context);
}

KTp::Message KTp::MessageProcessor::processIncomingMessage(const Tp::ReceivedMessage &message, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel)
{
    KTp::MessageContext context(account, channel);
    return processIncomingMessage(KTp::Message(message, context), context);
}

KTp::Message MessageProcessor::processIncomingMessage(KTp::Message message, const KTp::MessageContext &context)
{
    Q_FOREACH (const FilterPlugin &plugin, d->filters) {
        kDebug() << "running filter :" << plugin.instance->metaObject()->className();
        plugin.instance->filterMessage(message, context);
    }
    return message;
}

KTp::OutgoingMessage MessageProcessor::processOutgoingMessage(const QString &messageText, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel)
{
    KTp::MessageContext context(account, channel);
    KTp::OutgoingMessage message(messageText);

    Q_FOREACH (const FilterPlugin &plugin, d->filters) {
        kDebug() << "running outgoing filter : " << plugin.instance->metaObject()->className();
        plugin.instance->filterOutgoingMessage(message, context);
    }

    return message;
}
