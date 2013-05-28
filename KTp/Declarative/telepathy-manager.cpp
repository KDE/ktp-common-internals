/*
    Copyright (C) 2013 David Edmundson <davidedmundson@kde.org>
    Copyright (C) 2013 Aleix Pol <aleixpol@kde.org>

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

#include "telepathy-manager.h"
#include <KTp/actions.h>

#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/AbstractClient>
#include <TelepathyQt/TextChannel>

#include <QDeclarativeEngine>

TelepathyManager::TelepathyManager(QObject *parent)
    : QObject(parent)
{
    Tp::registerTypes();

    m_accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                  Tp::Features() << Tp::Account::FeatureCore
                                                  << Tp::Account::FeatureProfile);

    m_connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus(),
                                                        Tp::Features() << Tp::Connection::FeatureCore);

    m_contactFactory = KTp::ContactFactory::create(Tp::Features()  << Tp::Contact::FeatureAlias
                                                   << Tp::Contact::FeatureSimplePresence
                                                   << Tp::Contact::FeatureCapabilities);

    m_channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());

    m_accountManager = Tp::AccountManager::create(m_accountFactory, m_connectionFactory, m_channelFactory, m_contactFactory);
}

TelepathyManager::~TelepathyManager()
{
}

Tp::AccountManagerPtr TelepathyManager::accountManager()
{
    return m_accountManager;
}

void TelepathyManager::becomeReady()
{
    m_accountManager->becomeReady();
}

bool TelepathyManager::registerClient(QObject *client, const QString &name)
{
    Tp::AbstractClient* abstractClient = dynamic_cast<Tp::AbstractClient*>(client);
    if (!abstractClient) {
        return false;
    }

    if (! m_clientRegistrar) {
        m_clientRegistrar = Tp::ClientRegistrar::create(m_accountManager);
    }

    //the client registrar will delete the handler when the registrar is deleted.
    QDeclarativeEngine::setObjectOwnership(client, QDeclarativeEngine::CppOwnership);

    return m_clientRegistrar->registerClient(Tp::AbstractClientPtr(abstractClient), name);
}

bool TelepathyManager::unregisterClient(QObject* client)
{
    Tp::AbstractClient* abstractClient = dynamic_cast<Tp::AbstractClient*>(client);
    return abstractClient && m_clientRegistrar && m_clientRegistrar->unregisterClient(Tp::AbstractClientPtr(abstractClient));
}

void TelepathyManager::addTextChatFeatures()
{
    m_connectionFactory->addFeatures(Tp::Features() << Tp::Connection::FeatureSelfContact);

    Tp::Features textFeatures = Tp::Features() << Tp::TextChannel::FeatureMessageQueue
                                            << Tp::TextChannel::FeatureMessageSentSignal
                                            << Tp::TextChannel::FeatureChatState
                                            << Tp::TextChannel::FeatureMessageCapabilities;

    m_contactFactory->addFeatures(Tp::Features() << Tp::Contact::FeatureAlias
                                  << Tp::Contact::FeatureSimplePresence
                                  << Tp::Contact::FeatureCapabilities
                                  << Tp::Contact::FeatureAvatarData);

    m_channelFactory->addFeaturesForTextChats(textFeatures);
    m_channelFactory->addFeaturesForTextChatrooms(textFeatures);
}

void TelepathyManager::addContactListFeatures()
{
    m_connectionFactory->addFeatures(Tp::Features() << Tp::Connection::FeatureRosterGroups
                                     << Tp::Connection::FeatureRoster
                                     << Tp::Connection::FeatureSelfContact);

    m_contactFactory->addFeatures(Tp::Features() << Tp::Contact::FeatureAlias
                                  << Tp::Contact::FeatureSimplePresence
                                  << Tp::Contact::FeatureCapabilities
                                  << Tp::Contact::FeatureAvatarData);

    //used for optionally keeping track of unread message count/icon in contact list
    Tp::Features textFeatures = Tp::Features() << Tp::TextChannel::FeatureMessageQueue;
    m_channelFactory->addFeaturesForTextChats(textFeatures);
}

void TelepathyManager::addAllFeatures()
{
    addTextChatFeatures();
    addContactListFeatures();
}

void TelepathyManager::openLogViewer(const Tp::AccountPtr &account, const KTp::ContactPtr &contact)
{
    KTp::Actions::openLogViewer(account, contact);
}

Tp::PendingChannelRequest* TelepathyManager::startAudioCall(const Tp::AccountPtr &account, const KTp::ContactPtr &contact)
{
    return KTp::Actions::startAudioCall(account, contact);
}

Tp::PendingChannelRequest* TelepathyManager::startAudioVideoCall(const Tp::AccountPtr &account, const KTp::ContactPtr &contact)
{
    return KTp::Actions::startAudioVideoCall(account, contact);
}

Tp::PendingChannelRequest* TelepathyManager::startChat(const Tp::AccountPtr &account, const KTp::ContactPtr &contact, bool delegateToPreferredHandler)
{
    return KTp::Actions::startChat(account, contact, delegateToPreferredHandler);
}

Tp::PendingChannelRequest *TelepathyManager::startChat(const Tp::AccountPtr &account, const KTp::ContactPtr &contact, const QString &preferredHandler)
{
    return account->ensureTextChat(contact, QDateTime::currentDateTime(), preferredHandler);
}

Tp::PendingOperation* TelepathyManager::startFileTransfer(const Tp::AccountPtr &account, const KTp::ContactPtr &contact, const QUrl &url)
{
    return KTp::Actions::startFileTransfer(account, contact, url);
}




