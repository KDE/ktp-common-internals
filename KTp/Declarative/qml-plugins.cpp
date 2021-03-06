/*
    Copyright (C) 2011  Lasath Fernando <kde@lasath.org>
    Copyright (C) 2013  Dan Vrátil <dvratil@redhat.com>
    Copyright (C) 2013  Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "qml-plugins.h"

#include <QQmlEngine>
#include <QQmlContext>

#include "conversation.h"
#include "conversations-model.h"
#include "messages-model.h"
#include "pinned-contacts-model.h"
#include "contact-pin.h"
#include "filtered-pinned-contacts-proxy-model.h"
#include "telepathy-manager.h"
#include "mainlogmodel.h"

#include <TelepathyQt/PendingChannelRequest>
#include "KTp/types.h"
#include "KTp/global-presence.h"
#include "KTp/Models/presence-model.h"
#include "KTp/Models/contacts-filter-model.h"
#include "KTp/Models/contacts-model.h"
#include "KTp/Models/accounts-list-model.h"

#include <QtQml>

void QmlPlugins::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri)

    engine->rootContext()->setContextProperty(QLatin1String("telepathyManager"), new TelepathyManager(engine));
}

void QmlPlugins::registerTypes(const char *uri)
{
    qmlRegisterType<KTp::ContactsModel> (uri, 0, 1, "ContactsModel");
    qmlRegisterType<KTp::AccountsListModel> (uri, 0, 1, "AccountsListModel");

    qmlRegisterType<ConversationsModel> (uri, 0, 1, "ConversationsModel");
    qmlRegisterType<PinnedContactsModel>(uri, 0, 1, "PinnedContactsModel");
    qmlRegisterType<ContactPin>(uri, 0, 1, "ContactPin");
    qmlRegisterType<FilteredPinnedContactsProxyModel>(uri, 0, 1, "FilteredPinnedContactsProxyModel");
    qmlRegisterType<KTp::GlobalPresence> (uri, 0, 1, "GlobalPresence");
    qmlRegisterType<KTp::PresenceModel> (uri, 0, 1, "PresenceModel");
    qmlRegisterType<MainLogModel> (uri, 0, 1, "MainLogModel");

    qmlRegisterUncreatableType<MessagesModel> (uri, 0, 1, "MessagesModel",
        QLatin1String("It will be created once the conversation is created"));

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qmlRegisterType<TelepathyManager>();
#else
    qmlRegisterAnonymousType<TelepathyManager>(uri, 0);
#endif
    qmlRegisterUncreatableType<ConversationsModel>(uri, 0, 1, "Conversation",
        QStringLiteral("Conversation type cannot be created from QML"));
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qmlRegisterType<Tp::PendingChannelRequest>();
    qmlRegisterType<Tp::PendingOperation>();
#else
    qmlRegisterAnonymousType<Tp::PendingChannelRequest>(uri, 0);
    qmlRegisterAnonymousType<Tp::PendingOperation>(uri, 0);
#endif
    qRegisterMetaType<Tp::Presence>();
    qRegisterMetaType<KTp::Presence>();
    qRegisterMetaType<Tp::AccountManagerPtr>();
    qRegisterMetaType<KTp::ContactPtr>();
    qRegisterMetaType<Tp::AccountPtr>();
    qRegisterMetaType<Tp::AccountSetPtr>();

    QMetaType::registerComparators<KTp::Presence>();
}

