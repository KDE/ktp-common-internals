/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
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

#include "controller.h"

#include "account.h"
#include "storage.h"

#include <KDebug>

#include <TelepathyQt4/PendingReady>

Controller::Controller(QObject *parent)
 : QObject(parent),
   m_storage(new Storage(this))
{
    // Set up the Factories.
    Tp::Features fAccountFactory;
    fAccountFactory << Tp::Account::FeatureCore
                    << Tp::Account::FeatureAvatar
                    << Tp::Account::FeatureCapabilities
                    << Tp::Account::FeatureProfile
                    << Tp::Account::FeatureProtocolInfo;
    Tp::AccountFactoryConstPtr accountFactory = Tp::AccountFactory::create(
            QDBusConnection::sessionBus(),
            fAccountFactory);

    Tp::Features fConnectionFactory;
    fConnectionFactory << Tp::Connection::FeatureCore
                       << Tp::Connection::FeatureRoster
                       << Tp::Connection::FeatureRosterGroups
                       << Tp::Connection::FeatureSelfContact
                       << Tp::Connection::FeatureSimplePresence;
    Tp::ConnectionFactoryConstPtr connectionFactory = Tp::ConnectionFactory::create(
            QDBusConnection::sessionBus(),
            fConnectionFactory);

    Tp::ChannelFactoryConstPtr channelFactory = Tp::ChannelFactory::create(
            QDBusConnection::sessionBus());

    Tp::Features fContactFactory;
    fContactFactory << Tp::Contact::FeatureAlias
                    << Tp::Contact::FeatureCapabilities
                    << Tp::Contact::FeatureInfo
                    << Tp::Contact::FeatureLocation
                    << Tp::Contact::FeatureSimplePresence;
    Tp::ContactFactoryConstPtr contactFactory = Tp::ContactFactory::create(fContactFactory);

    // Create an instance of the AccountManager and start to get it ready.
    m_accountManager = Tp::AccountManager::create(accountFactory,
                                                  connectionFactory,
                                                  channelFactory,
                                                  contactFactory);

    connect(m_accountManager->becomeReady(Tp::Features() << Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    kDebug() << "Calling becomeReady on the AM.";
}

Controller::~Controller()
{

}

void Controller::onAccountManagerReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Account manager cannot become ready:"
                   << op->errorName()
                   << op->errorMessage();
        return;
    }

    kDebug() << "AccountManager ready.";

     // Account Manager is now ready. We should watch for any new accounts being created.
    connect(m_accountManager.data(),
            SIGNAL(newAccount(const Tp::AccountPtr &)),
            SLOT(onNewAccount(const Tp::AccountPtr &)));

    // Take into account (ha ha) the accounts that already existed when the AM object became ready.
    foreach (const Tp::AccountPtr &account, m_accountManager->allAccounts()) {
        onNewAccount(account);
    }
}

void Controller::onNewAccount(const Tp::AccountPtr &account)
{
    Account *acc = new Account(account, this);

    kDebug() << "Created new account: " << acc;

    // Connect to the signals/slots that signify the account changing in some way.
    connect(acc, SIGNAL(created(QString,QString,QString)),
            m_storage, SLOT(createAccount(QString,QString,QString)));
    connect(acc, SIGNAL(accountDestroyed(QString)),
            m_storage, SLOT(destroyAccount(QString)));
    connect(acc, SIGNAL(nicknameChanged(QString,QString)),
            m_storage, SLOT(setAccountNickname(QString,QString)));
    connect(acc, SIGNAL(currentPresenceChanged(QString,Tp::SimplePresence)),
            m_storage, SLOT(setAccountCurrentPresence(QString,Tp::SimplePresence)));

    // Connect to all the signals/slots that signify the contacts are changing in some way.
    connect(acc, SIGNAL(contactCreated(QString,QString)),
            m_storage, SLOT(createContact(QString,QString)));
    connect(acc, SIGNAL(contactDestroyed(QString,QString)),
            m_storage, SLOT(destroyContact(QString,QString)));
    connect(acc, SIGNAL(contactAliasChanged(QString,QString,QString)),
            m_storage, SLOT(setContactAlias(QString,QString,QString)));
    connect(acc, SIGNAL(contactPresenceChanged(QString,QString,Tp::SimplePresence)),
            m_storage, SLOT(setContactPresence(QString,QString,Tp::SimplePresence)));
    connect(acc, SIGNAL(contactAddedToGroup(QString,QString,QString)),
            m_storage, SLOT(addContactToGroup(QString,QString,QString)));
    connect(acc, SIGNAL(contactRemovedFromGroup(QString,QString,QString)),
            m_storage, SLOT(removeContactFromGroup(QString,QString,QString)));
    connect(acc, SIGNAL(contactBlockStatusChanged(QString,QString,bool)),
            m_storage, SLOT(setContactBlockStatus(QString,QString,bool)));
    connect(acc, SIGNAL(contactPublishStateChanged(QString,QString,Tp::Contact::PresenceState)),
            m_storage, SLOT(setContactPublishState(QString,QString,Tp::Contact::PresenceState)));
    connect(acc, SIGNAL(contactSubscriptionStateChanged(QString,QString,Tp::Contact::PresenceState)),
            m_storage, SLOT(setContactSubscriptionState(QString,QString,Tp::Contact::PresenceState)));

    // Now the signal connections are done, initialise the account.
    acc->init();
}


#include "controller.moc"

