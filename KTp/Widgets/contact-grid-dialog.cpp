/*
 * Contact Chooser Dialog
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2012 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
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


#include "contact-grid-dialog.h"

#include <KDE/KLineEdit>
#include <KDE/KPushButton>
#include <KDE/KLocalizedString>
#include <KDE/KDebug>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/PendingReady>

#include <KTp/debug.h>
#include <KTp/Models/accounts-model.h>
#include <KTp/Models/accounts-filter-model.h>
#include <KTp/Widgets/contact-grid-widget.h>
#include <telepathy-qt4/TelepathyQt/PendingChannelRequest>



KTp::ContactGridDialog::ContactGridDialog(QWidget *parent) :
    KDialog(parent),
    m_accountsModel(0),
    m_contact(0)
{
    resize(500,450);

    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                       Tp::Features() << Tp::Account::FeatureCore
                                                                                      << Tp::Account::FeatureAvatar
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

    m_accountsModel = new AccountsModel(this);
    connect(m_accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)), SLOT(onAccountManagerReady()));


    m_contactGridWidget = new KTp::ContactGridWidget(m_accountsModel, this);
    m_contactGridWidget->contactFilterLineEdit()->setClickMessage(i18n("Search in Contacts..."));
    m_contactGridWidget->filter()->setPresenceTypeFilterFlags(AccountsFilterModel::ShowOnlyConnected);
//    m_contactGridWidget->filter()->setCapabilityFilterFlags(AccountsFilterModel::FilterBySSHContactCapability);
    setMainWidget(m_contactGridWidget);

    connect(m_contactGridWidget,
            SIGNAL(selectionChanged(Tp::AccountPtr,Tp::ContactPtr)),
            SLOT(onChanged()));

    button(KDialog::Ok)->setDisabled(true);

    connect(this, SIGNAL(okClicked()), SLOT(onOkClicked()));
    connect(this, SIGNAL(rejected()), SLOT(close()));
}

Tp::AccountPtr KTp::ContactGridDialog::account()
{
    return m_account;
}

Tp::ContactPtr KTp::ContactGridDialog::contact()
{
    return m_contact;
}

void KTp::ContactGridDialog::onAccountManagerReady()
{
    kDebug() << "Account manager is ready";
    m_accountsModel->setAccountManager(m_accountManager);
}

void KTp::ContactGridDialog::onOkClicked()
{
    // don't do anytghing if no contact has been selected
    if (!m_contactGridWidget->hasSelection()) {
        // show message box?
        return;
    }

    m_contact = m_contactGridWidget->selectedContact();
    m_account = m_contactGridWidget->selectedAccount();

    if (m_account.isNull()) {
        kWarning() << "Account is NULL";
    } else if (m_contact.isNull()) {
        kWarning() << "Contact is NULL";
    } else {
        kDebug() << "Account is: " << m_account->displayName();
        kDebug() << "Contact is: " << m_contact->alias();
    }
}

void KTp::ContactGridDialog::onChanged()
{
    button(KDialog::Ok)->setEnabled(m_contactGridWidget->hasSelection());
}
