/*
    Copyright (C) 2011  David Edmundson <kde@davidedmundson.co.uk>

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

#include "contact-list.h"

#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/ConnectionFactory>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingChannelRequest>

#include <KDebug>

#include <KTp/contact-factory.h>

#include <KTp/actions.h>
#include <Models/contacts-model.h>
#include <Models/contacts-list-model.h>

ContactList::ContactList(QObject *parent)
    : QObject(parent),
      m_filterModel(new KTp::ContactsModel(this))
{
    Tp::registerTypes();

    // Start setting up the Telepathy AccountManager.
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
                                                                      << Tp::Contact::FeatureCapabilities);



    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());

    Tp::AccountManagerPtr accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(),
                                                  accountFactory,
                                                  connectionFactory,
                                                  channelFactory,
                                                  contactFactory);

    m_filterModel->setAccountManager(accountManager);
}

KTp::ContactsModel* ContactList::filterModel() const
{
    return m_filterModel;
}

void ContactList::startChat(const Tp::AccountPtr &account, const KTp::ContactPtr &contact)
{
    kDebug() << "Requesting chat for contact" << contact->alias();
    kDebug() << "account is" << account->normalizedName();

    Tp::PendingOperation *op = KTp::Actions::startChat(account, contact, false);
    connect(op, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

void ContactList::onGenericOperationFinished(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kDebug() << op->errorName();
        kDebug() << op->errorMessage();
    }
}
