/*
 * This file is part of telepathy-integration-daemon
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
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

extern "C"
{
#include <signal.h>
}

#include "telepathyaccountmonitor.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>
#include <KUniqueApplication>

#include <Nepomuk/ResourceManager>

namespace
{
    static void signal_handler(int signal)
    {
        if ((signal == SIGTERM) || (signal == SIGINT)) {
            QCoreApplication * const app(QCoreApplication::instance());
            if (app != 0) {
                app->quit();
            }
        }
    }
}

int main(int argc, char *argv[])
{
    KAboutData aboutData("telepathy-integration-daemon", 0, ki18n("Telepathy Integration Daemon"), "0.1");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KUniqueApplication app;

    Nepomuk::ResourceManager::instance()->init();

    // Create an instance of the Telepathy Account Monitor.
    TelepathyAccountMonitor *monitor = new TelepathyAccountMonitor(&app);

    // Set up signal handlers.
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        kWarning() << "Setting up SIGINT signal handler failed.";
    }

    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        kWarning() << "Setting up SIGTERM signal handler failed.";
    }

    // Quite the application when the monitor is destroyed.
    QObject::connect(monitor, SIGNAL(destroyed()), &app, SLOT(quit()));

    kDebug() << "Let's go...";

    // Start event loop.
    app.exec();
}

