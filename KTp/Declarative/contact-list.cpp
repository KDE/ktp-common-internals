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
#include <TelepathyQt/ContactFactory>
#include <TelepathyQt/ConnectionFactory>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingReady>

#include <KDebug>

#include <KTp/Models/contact-model-item.h>
#include <KTp/Models/accounts-model-item.h>

#define PREFERRED_TEXTCHAT_HANDLER "org.freedesktop.Telepathy.Client.KTp.TextUi"

ContactList::ContactList(QObject *parent)
    : QObject(parent),
      m_accountsModel(new AccountsModel(this)),
      m_filterModel(new AccountsFilterModel(this)),
      m_flatModel(0)
{
    m_filterModel->setSourceModel(m_accountsModel);
    //flat model takes the source as a constructor parameter, the other's don't.
    //due to a bug somewhere creating the flat model proxy with the filter model as a source before the filter model has a source means the rolenames do not get propgated up
    m_flatModel = new FlatModelProxy(m_filterModel);

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

    Tp::ContactFactoryPtr contactFactory = Tp::ContactFactory::create(Tp::Features()  << Tp::Contact::FeatureAlias
                                                                      << Tp::Contact::FeatureAvatarData
                                                                      << Tp::Contact::FeatureSimplePresence
                                                                      << Tp::Contact::FeatureCapabilities);



    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());

    m_accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(),
                                                  accountFactory,
                                                  connectionFactory,
                                                  channelFactory,
                                                  contactFactory);

    m_filterModel->setDynamicSortFilter(true);

    connect(m_accountManager->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}


void ContactList::onAccountManagerReady(Tp::PendingOperation *op)
{
    Q_UNUSED(op);
    m_accountsModel->setAccountManager(m_accountManager);
}

FlatModelProxy * ContactList::flatModel() const
{
    return m_flatModel;
}

AccountsFilterModel * ContactList::filterModel() const
{
    return m_filterModel;
}

void ContactList::startChat(ContactModelItem *contactItem)
{
    
    Tp::ContactPtr contact = contactItem->contact();

    kDebug() << "Requesting chat for contact" << contact->alias();
    Tp::AccountPtr account = m_accountsModel->accountForContactItem(contactItem);

    Tp::ChannelRequestHints hints;
    hints.setHint("org.freedesktop.Telepathy.ChannelRequest","DelegateToPreferredHandler", QVariant(true));

    Tp::PendingChannelRequest *channelRequest = account->ensureTextChat(contact,
                                                                        QDateTime::currentDateTime(),
                                                                        PREFERRED_TEXTCHAT_HANDLER,
                                                                        hints);
}

