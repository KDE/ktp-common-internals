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

class MessageProcessor::Private
{
  public:
    Private(MessageProcessor *parent):
        q(parent)
    { }

    void loadFilters();

    QList<KTp::AbstractMessageFilter*> filters;

  private:
    MessageProcessor *q;
};

bool pluginWeightLessThan(const KPluginInfo &p1, const KPluginInfo &p2)
{
    bool ok;
    int weight1 = p1.service()->property(QLatin1String("X-KDE-PluginInfo-Weight"), QVariant::Int).toInt(&ok);
    if (!ok) {
        weight1 = 100;
    }
    int weight2 = p2.service()->property(QLatin1String("X-KDE-PluginInfo-Weight"), QVariant::Int).toInt(&ok);
    if (!ok) {
        weight2 = 100;
    }

    return weight1 < weight2;
}

void MessageProcessor::Private::loadFilters()
{
    kDebug() << "Starting loading filters...";

    KPluginInfo::List plugins = MessageFilterConfigManager::self()->enabledPlugins();

    qSort(plugins.begin(), plugins.end(), pluginWeightLessThan);

    Q_FOREACH (const KPluginInfo &plugin, plugins) {
        KService::Ptr service = plugin.service();

        KPluginFactory *factory = KPluginLoader(service->library()).factory();
        if(factory) {
            kDebug() << "loaded factory :" << factory;
            AbstractMessageFilter *filter = factory->create<AbstractMessageFilter>(q);

            if(filter) {
                kDebug() << "loaded message filter : " << filter;
                filters.append(filter);
            }
        } else {
            kError() << "error loading plugin :" << service->library();
        }
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
    d->filters.append(new MessageEscapeFilter(this));
    d->filters.append(new MessageUrlFilter(this));

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
    Q_FOREACH (AbstractMessageFilter *filter, d->filters) {
        Q_FOREACH (const QString &script, filter->requiredScripts()) {
            // Avoid duplicates
            if (!scripts.contains(script)) {
                scripts << script;
            }
        }
        Q_FOREACH (const QString &stylesheet, filter->requiredStylesheets()) {
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

KTp::Message KTp::MessageProcessor::processIncomingMessage(const Tpl::TextEventPtr &message, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel)
{
    KTp::MessageContext context(account, channel);
    return processIncomingMessage(KTp::Message(message, context), context);
}


KTp::Message MessageProcessor::processIncomingMessage(KTp::Message message, const KTp::MessageContext &context)
{
    Q_FOREACH (AbstractMessageFilter *filter, d->filters) {
        kDebug() << "running filter :" << filter->metaObject()->className();
        filter->filterMessage(message, context);
    }
    return message;
}

KTp::OutgoingMessage MessageProcessor::processOutgoingMessage(const QString &messageText, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel)
{
    KTp::MessageContext context(account, channel);
    KTp::OutgoingMessage message(messageText);

    Q_FOREACH (AbstractMessageFilter *filter, d->filters) {
        kDebug() << "running outgoing filter : " << filter->metaObject()->className();
        filter->filterOutgoingMessage(message, context);
    }

    return message;
}
