/*
 * This file is part of telepathy-kde-models-test-ui
 *
 * Copyright (C) 2011 Collabora Ltd. <info@collabora.co.uk>
 *   @Author George Goldberg <george.goldberg@collabora.co.uk>
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
 */

#include "model-view.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>
#include <KApplication>

#include <TelepathyQt/Types>
#include <TelepathyQt/Debug>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/ConnectionFactory>
#include <TelepathyQt/TextChannel>

#include <QDBusConnection>

#include "KTp/Models/contacts-list-model.h"
#include "KTp/contact-factory.h"

int main(int argc, char *argv[])
{
    KAboutData aboutData("telepathy-kde-models-test-ui",
                         0,
                         ki18n("Telepathy KDE Models Test UI"),
                         "0.1",
                         ki18n("Telepathy KDE Models Test UI"),
                         KAboutData::License_LGPL,
                         ki18n("(C) 2011 Collabora Ltd"));

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;

    Tp::registerTypes();
    Tp::enableDebug(false);
    Tp::enableWarnings(true);

    
        Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                       Tp::Features() << Tp::Account::FeatureCore
                                                                       << Tp::Account::FeatureAvatar
                                                                       << Tp::Account::FeatureCapabilities
                                                                       << Tp::Account::FeatureProtocolInfo
                                                                       << Tp::Account::FeatureProfile);

    Tp::ConnectionFactoryPtr connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus(),
                                                                               Tp::Features() << Tp::Connection::FeatureCore
                                                                               << Tp::Connection::FeatureRosterGroups
                                                                               << Tp::Connection::FeatureRoster
                                                                               << Tp::Connection::FeatureSelfContact);

    Tp::ContactFactoryPtr contactFactory = KTp::ContactFactory::create(Tp::Features()  << Tp::Contact::FeatureAlias
                                                                      << Tp::Contact::FeatureAvatarData
                                                                      << Tp::Contact::FeatureSimplePresence
                                                                      << Tp::Contact::FeatureCapabilities
                                                                      << Tp::Contact::FeatureClientTypes);

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addFeaturesForTextChats(Tp::Features() << Tp::Channel::FeatureCore << Tp::TextChannel::FeatureMessageQueue);

    Tp::AccountManagerPtr accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(),
                                                  accountFactory,
                                                  connectionFactory,
                                                  channelFactory,
                                                  contactFactory);

    KTp::ContactsListModel *model = new KTp::ContactsListModel(&app);
    model->setAccountManager(accountManager);
    
    // Set up and show the main widget
    ModelView *mainWidget = new ModelView(model, 0);
    mainWidget->show();

    // Start event loop.
    app.exec();
}

