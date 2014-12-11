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

#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingChannelRequest>

#include <KTp/debug.h>
#include <KTp/Models/contacts-list-model.h>
#include <KTp/Models/contacts-filter-model.h>
#include <KTp/Widgets/contact-grid-widget.h>
#include <KTp/contact-factory.h>

class KTp::ContactGridDialog::Private
{
public:
    Private(KTp::ContactGridDialog *parent) :
        q(parent),
        contactsModel(0)
    {
    }

    KTp::ContactGridDialog * const q;

    Tp::AccountManagerPtr accountManager;
    KTp::ContactsListModel *contactsModel;
    KTp::ContactGridWidget *contactGridWidget;
    QDialogButtonBox *buttonBox;

public:
    void _k_onAccountManagerReady();
    void _k_onSelectionChanged();
};

void KTp::ContactGridDialog::Private::_k_onAccountManagerReady()
{
    contactsModel->setAccountManager(accountManager);
}

void KTp::ContactGridDialog::Private::_k_onSelectionChanged()
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(contactGridWidget->hasSelection());
}


KTp::ContactGridDialog::ContactGridDialog(QWidget *parent) :
    QDialog(parent),
    d(new Private(this))
{
    resize(500,450);

    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                       Tp::Features() << Tp::Account::FeatureCore
                                                                                      << Tp::Account::FeatureAvatar
                                                                                      << Tp::Account::FeatureProtocolInfo
                                                                                      << Tp::Account::FeatureProfile
                                                                                      << Tp::Account::FeatureCapabilities);

    Tp::ConnectionFactoryPtr connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus(),
                                                                               Tp::Features() << Tp::Connection::FeatureCore
                                                                                              << Tp::Connection::FeatureRosterGroups
                                                                                              << Tp::Connection::FeatureRoster
                                                                                              << Tp::Connection::FeatureSelfContact);

    Tp::ContactFactoryPtr contactFactory = KTp::ContactFactory::create(Tp::Features() << Tp::Contact::FeatureAlias
                                                                                      << Tp::Contact::FeatureAvatarData
                                                                                      << Tp::Contact::FeatureSimplePresence
                                                                                      << Tp::Contact::FeatureCapabilities);

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());

    d->accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(),
                                                   accountFactory,
                                                   connectionFactory,
                                                   channelFactory,
                                                   contactFactory);

    d->contactsModel = new KTp::ContactsListModel(this);
    connect(d->accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)), SLOT(_k_onAccountManagerReady()));


    d->contactGridWidget = new KTp::ContactGridWidget(d->contactsModel, this);
    d->contactGridWidget->contactFilterLineEdit()->setPlaceholderText(i18n("Search in Contacts..."));
    d->contactGridWidget->filter()->setPresenceTypeFilterFlags(KTp::ContactsFilterModel::ShowOnlyConnected);

    d->buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(d->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(d->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));


    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(d->contactGridWidget);
    mainLayout->addWidget(d->buttonBox);

    setLayout(mainLayout);

    connect(d->contactGridWidget,
            SIGNAL(contactDoubleClicked(Tp::AccountPtr,KTp::ContactPtr)),
            SLOT(accept()));
    connect(d->contactGridWidget,
            SIGNAL(selectionChanged(Tp::AccountPtr,KTp::ContactPtr)),
            SLOT(_k_onSelectionChanged()));

    d->_k_onSelectionChanged();

    connect(this, SIGNAL(rejected()), SLOT(close()));
}

KTp::ContactGridDialog::~ContactGridDialog()
{
    delete d;
}

Tp::AccountPtr KTp::ContactGridDialog::account()
{
    return d->contactGridWidget->selectedAccount();
}

Tp::ContactPtr KTp::ContactGridDialog::contact()
{
    return d->contactGridWidget->selectedContact();
}

KTp::ContactsFilterModel* KTp::ContactGridDialog::filter() const
{
    return d->contactGridWidget->filter();
}


#include "moc_contact-grid-dialog.cpp"
