/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "proxy-service.h"
#include "otr-config.h"
#include "ktp_version.h"
#include "ktp-proxy-debug.h"

#include <KTp/OTR/types.h>

#include "KTp/core.h"
#include "KTp/debug.h"

#include <QDBusConnection>
#include <QCoreApplication>

#include <TelepathyQt/AbstractAdaptor>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/TextChannel>

extern "C" {
#include <libotr/proto.h>
}


Tp::ChannelFactoryPtr getChannelFactory()
{
    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);

    Tp::Features textFeatures = Tp::Features() << Tp::TextChannel::FeatureMessageQueue
                                               << Tp::TextChannel::FeatureMessageSentSignal
                                               << Tp::TextChannel::FeatureChatState
                                               << Tp::TextChannel::FeatureMessageCapabilities;
    channelFactory->addFeaturesForTextChats(textFeatures);

    return channelFactory;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if(QCoreApplication::arguments().contains(QLatin1String("--debug"))) {
        KTp::Debug::installCallback(true, true);
    }

    Tp::registerTypes();
    KTp::registerOtrTypes();
    OTRL_INIT;

    OTR::Config config;

    Tp::DBusError error;

    QDBusConnection dbusConnection = QDBusConnection::sessionBus();
    Tp::ClientRegistrarPtr registrar = Tp::ClientRegistrar::create(
            KTp::accountFactory(),
            KTp::connectionFactory(),
            getChannelFactory(),
            KTp::contactFactory());

    ProxyService ps(dbusConnection, &config, registrar);
    ps.registerService(&error);

    if(error.isValid())
    {
        qCCritical(KTP_PROXY) << "Could not register ProxyService\n"
            << "error name: " << error.name() << "\n"
            << "error message: " << error.message();

        return 1;
    } else {
        return app.exec();
    }
}
