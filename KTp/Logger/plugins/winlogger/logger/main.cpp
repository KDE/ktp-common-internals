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

#include <KTp/telepathy-handler-application.h>

#include <TelepathyQt/SharedPtr>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/ConnectionFactory>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ClientRegistrar>

int main(int argc, char **argv)
{
    KTp::TelepathyHandlerApplication app;

    Tp::SharedPtr<Handler> handler (new Handler(&app));

    // Setting up the Telepathy Client Registrar
    Tp::AccountFactoryPtr accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus());
    Tp::ConnectionFactoryPtr  connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus());
    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);
    channelFactory->addFeaturesForTextChatrooms(Tp::TextChannel::FeatureCore);
    channelFactory->addFeaturesForTextChatrooms(Tp::TextChannel::FeatureMessageQueue);
    channelFactory->addFeaturesForTextChats(Tp::TextChannel::FeatureCore);
    channelFactory->addFeaturesForTextChats(Tp::TextChannel::FeatureMessageQueue);

    Tp::ContactFactoryPtr contactFactory = Tp::ContactFactory::create();
    Tp::ClientRegistrarPtr registrar = Tp::ClientRegistrar::create(accountFactory,
                                                                   connectionFactory,
                                                                   channelFactory,
                                                                   contactFactory);
    if (!registrar->registerClient(Tp::AbstractClientPtr(handler), "KTp.WinLogger")) {
        return 1;
    }

    app.exec();
}
