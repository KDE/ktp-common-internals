/*
 * static methods on the KTp namespace
 *
 * Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
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

#include "core.h"

#ifdef HAVE_KPEOPLE
#include <QDBusMessage>
#include <QDBusReply>
#endif

#include <KGlobal>

#include <TelepathyQt/AccountManager>
#include <KTp/global-contact-manager.h>
#include "contact-factory.h"

class CorePrivate
{
public:
    CorePrivate();
    bool m_kPeopleEnabled;
    Tp::AccountManagerPtr m_accountManager;
    KTp::GlobalContactManager *m_contactManager;
};

CorePrivate::CorePrivate()
    : m_kPeopleEnabled(false),
      m_contactManager(0)
{
    //if built with kpeople support, enable it
    #ifdef HAVE_KPEOPLE
    m_kPeopleEnabled = true;
    #endif

    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                    Tp::Features() << Tp::Account::FeatureCore
                                                                                   << Tp::Account::FeatureCapabilities
                                                                                   << Tp::Account::FeatureProtocolInfo
                                                                                   << Tp::Account::FeatureProfile);

    Tp::ConnectionFactoryPtr connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus(),
                                                                               Tp::Features() << Tp::Connection::FeatureCore
                                                                                              << Tp::Connection::FeatureSelfContact);

    Tp::ContactFactoryPtr contactFactory = KTp::ContactFactory::create(Tp::Features()  << Tp::Contact::FeatureAlias
                                                                                       << Tp::Contact::FeatureSimplePresence
                                                                                       << Tp::Contact::FeatureCapabilities
                                                                                       << Tp::Contact::FeatureClientTypes
                                                                                       << Tp::Contact::FeatureAvatarData);

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());

    m_accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(),
                                                   accountFactory,
                                                   connectionFactory,
                                                   channelFactory,
                                                   contactFactory);
}

K_GLOBAL_STATIC(CorePrivate, s_instance)

bool KTp::kpeopleEnabled()
{
    return s_instance->m_kPeopleEnabled;
}

Tp::AccountManagerPtr KTp::accountManager()
{
    return s_instance->m_accountManager;
}

KTp::GlobalContactManager* KTp::contactManager()
{
    if (!s_instance->m_contactManager) {
        s_instance->m_contactManager = new KTp::GlobalContactManager(s_instance->m_accountManager, 0);
    }

    return s_instance->m_contactManager;
}
