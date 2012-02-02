/***************************************************************************
 *   Copyright (C) 2011 by Francesco Nwokeka <francesco.nwokeka@gmail.com> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#include "telepathyContactList.h"

#include <KStandardDirs>

#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>

#include <TelepathyQt4/AccountFactory>
#include <TelepathyQt4/ContactFactory>
#include <TelepathyQt4/ConnectionFactory>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/PendingReady>

#include "flat-model-proxy.h"

#include <KTelepathy/Models/accounts-model.h>



TelepathyContactList::TelepathyContactList(QObject* parent, const QVariantList& args)
    : Applet(parent, args)
    , m_declarative(new Plasma::DeclarativeWidget(this))
    , m_qmlObject(0)
{
    // set plasmoid size
    setMinimumSize(250, 400);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    
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

    connect(m_accountManager->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    m_model = new AccountsModel(m_accountManager, this);

    
}

TelepathyContactList::~TelepathyContactList()
{
    delete m_declarative;
}

int TelepathyContactList::appletHeight() const
{
    return geometry().height();
}

int TelepathyContactList::appletWidth() const
{
    return geometry().width();
}


void TelepathyContactList::init()
{
    // load QML part of the plasmoid
    if (m_declarative) {
        QString qmlFile = KGlobal::dirs()->findResource("data", "plasma/plasmoids/org.kde.telepathy-contact-list/contents/ui/main.qml");
        qDebug() << "LOADING: " << qmlFile;
        m_declarative->setQmlPath(qmlFile);

        // make C++ Plasma::Applet available to QML for resize signal
        m_declarative->engine()->rootContext()->setContextProperty("TelepathyContactList", this);

        FlatModelProxy *proxyModel = new FlatModelProxy(m_model);
        m_declarative->engine()->rootContext()->setContextProperty("contactListModel", proxyModel);

        // setup qml object so that we can talk to the declarative part
        m_qmlObject = dynamic_cast<QObject*>(m_declarative->rootObject());

        // connect the qml object to recieve signals from C++ end
        // these two signals are for the plasmoid resize. QML can't determine the Plasma::DeclarativeWidget's boundaries
        connect(this, SIGNAL(widthChanged()), m_qmlObject, SLOT(onWidthChanged()));
        connect(this, SIGNAL(heightChanged()), m_qmlObject, SLOT(onHeightChanged()));

        //FIXME this code is messy, steal from qmlsplashscreen
    }
}


// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(telepathy-contact-list, TelepathyContactList)

void TelepathyContactList::onAccountManagerReady(Tp::PendingOperation *op)
{
    m_model->init();
}
