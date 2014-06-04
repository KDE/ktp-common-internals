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
#include <KLocalizedString>

#include <TelepathyQt/Types>
#include <TelepathyQt/Debug>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/ConnectionFactory>
#include <TelepathyQt/TextChannel>

#include <QDBusConnection>
#include <QApplication>

#include "KTp/Models/contacts-list-model.h"
#include "KTp/contact-factory.h"
#include "KTp/core.h"


int main(int argc, char *argv[])
{
    KAboutData aboutData(QStringLiteral("telepathy-kde-models-test-ui"),
                         i18n("Telepathy KDE Models Test UI"),
                         QStringLiteral("0.1"),
                         i18n("Telepathy KDE Models Test UI"),
                         KAboutLicense::LGPL,
                         i18n("(C) 2011 Collabora Ltd"));

    KAboutData::setApplicationData(aboutData);
    QApplication app(argc, argv);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    Tp::registerTypes();
    Tp::enableDebug(false);
    Tp::enableWarnings(true);

    
    const Tp::AccountManagerPtr accountManager = KTp::accountManager();

    KTp::ContactsListModel *model = new KTp::ContactsListModel(&app);
    model->setAccountManager(accountManager);
    
    // Set up and show the main widget
    ModelView *mainWidget = new ModelView(model, 0);
    mainWidget->show();

    // Start event loop.
    app.exec();
}

