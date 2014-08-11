/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
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

#include "proxy-service.h"
#include "otr-config.h"
#include "version.h"
#include "types.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>
#include <KDebug>

#include <QDBusConnection>

#include <TelepathyQt/AbstractAdaptor>
#include <TelepathyQt/Channel>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/TextChannel>

extern "C" {
#include <libotr/proto.h>
}


int main(int argc, char *argv[])
{
    KAboutData aboutData("ktp-proxy", 0,
                         ki18n("Channel proxy service"),
                         KTP_PROXY_VERSION);

    aboutData.addAuthor(ki18n("Marcin Ziemiński"), ki18n("Developer"), "zieminn@gmail.com");
    aboutData.setProductName("telepathy/ktp-proxy");
    aboutData.setLicense(KAboutData::License_GPL_V2);
    aboutData.setProgramIconName(QLatin1String("telepathy-kde"));

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app(false);

    Tp::registerTypes();
    Tp::registerProxyTypes();
    OTRL_INIT;

    OTR::Config config;

    Tp::DBusError error;
    QDBusConnection dbusConnection = QDBusConnection::sessionBus();
    ProxyService ps(dbusConnection, &config);
    ps.registerService(&error);

    if(error.isValid())
    {
        kError() << "Could not register ProxyService\n"
            << "error name: " << error.name() << "\n"
            << "error message: " << error.message();

        return 1;
    } else {
        return app.exec();
    }
}
