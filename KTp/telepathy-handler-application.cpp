/*
* Copyright (C) 2011 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
* Copyright (C) 1999 Preston Brown <pbrown@kde.org>
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

#include "telepathy-handler-application.h"
#include "ktp-debug.h"

#include <cstdlib>

#include <QTimer>
#include <KLocalizedString>
#include "debug.h"

#include <TelepathyQt/Types>
#include <TelepathyQt/Debug>


extern bool kde_kdebug_enable_dbus_interface;

namespace KTp
{

class TelepathyHandlerApplication::Private
{
public:
    Private(TelepathyHandlerApplication *q);
    ~Private();

    void _k_onInitialTimeout();
    void _k_onTimeout();

    void init(int initialTimeout, int timeout);

    TelepathyHandlerApplication *q;

    static bool s_persist;
    static bool s_debug;

    int initialTimeout;
    int timeout;
    QTimer *timer;
    bool firstJobStarted;
    QAtomicInt jobCount;
};

TelepathyHandlerApplication::Private::Private(TelepathyHandlerApplication *q)
    : q(q),
      firstJobStarted(false),
      jobCount(0)
{
}

TelepathyHandlerApplication::Private::~Private()
{
}

void TelepathyHandlerApplication::Private::_k_onInitialTimeout()
{
    if (jobCount.load() == 0 && jobCount.fetchAndAddOrdered(-1) == 0) {
        // m_jobCount is now -1
        qDebug() << "No job received. Exiting";
        QCoreApplication::quit();
    }
}

void TelepathyHandlerApplication::Private::_k_onTimeout()
{
    if (jobCount.load() == 0 && jobCount.fetchAndAddOrdered(-1) == 0) {
        // m_jobCount is now -1
        qDebug() << "Timeout. Exiting";
        QCoreApplication::quit();
    }
}

void TelepathyHandlerApplication::Private::init(int initialTimeout, int timeout)
{
    this->initialTimeout = initialTimeout;
    this->timeout = timeout;

    // If timeout < 0 we let the application exit when the last window is closed,
    // Otherwise we handle it with the timeout
    if (timeout >= 0) {
        q->setQuitOnLastWindowClosed(false);
    }

    // Register TpQt4 types
    Tp::registerTypes();

    // Install TpQt4 debug callback
    KTp::Debug::installCallback(s_debug);

    if (!Private::s_persist) {
        timer = new QTimer(q);
        if (initialTimeout >= 0) {
            q->connect(timer, SIGNAL(timeout()), q, SLOT(_k_onInitialTimeout()));
            timer->start(initialTimeout);
        }
    }
}

bool TelepathyHandlerApplication::Private::s_persist = false;
bool TelepathyHandlerApplication::Private::s_debug = false;


TelepathyHandlerApplication::TelepathyHandlerApplication(int &argc, char *argv[],
                                                         int initialTimeout,
                                                         int timeout)
    : QApplication(argc, argv),
      d(new Private(this))
{
    QCommandLineOption persistOption(QStringLiteral("persist"), i18n("Persistent mode (do not exit on timeout)"));
    QCommandLineOption debugOption(QStringLiteral("debug"), i18n("Show Telepathy debugging information"));

    QCommandLineParser cmdParser;
    cmdParser.addHelpOption();
    cmdParser.addOption(persistOption);
    cmdParser.addOption(debugOption);
    cmdParser.process(qApp->arguments());

    Private::s_persist = cmdParser.isSet(QStringLiteral("persist"));
    Private::s_debug = cmdParser.isSet(QStringLiteral("debug"));


    d->init(initialTimeout, timeout);
}

TelepathyHandlerApplication::~TelepathyHandlerApplication()
{
    delete d;
}

int TelepathyHandlerApplication::newJob()
{
    TelepathyHandlerApplication *app = qobject_cast<TelepathyHandlerApplication*>(qApp);
    TelepathyHandlerApplication::Private *d = app->d;

    int ret = d->jobCount.fetchAndAddOrdered(1);
    if (!Private::s_persist) {
        if (d->timer->isActive()) {
            d->timer->stop();
        }
        if (!d->firstJobStarted) {
            if (d->initialTimeout) {
                disconnect(d->timer, SIGNAL(timeout()), app, SLOT(_k_onInitialTimeout()));
            }
            if (d->timeout >= 0) {
                connect(d->timer, SIGNAL(timeout()), app, SLOT(_k_onTimeout()));
            }
            d->firstJobStarted = true;
        }
    }
    qDebug() << "New job started." << d->jobCount.load() << "jobs currently running";
    return ret;
}

void TelepathyHandlerApplication::jobFinished()
{
    TelepathyHandlerApplication *app = qobject_cast<TelepathyHandlerApplication*>(qApp);
    TelepathyHandlerApplication::Private *d = app->d;

    if (d->jobCount.fetchAndAddOrdered(-1) <= 1) {
        if (!Private::s_persist && d->timeout >= 0) {
            qDebug() << "No other jobs at the moment. Starting timer.";
            d->timer->start(d->timeout);
        }
    }
    qDebug() << "Job finished." << d->jobCount.load() << "jobs currently running";
}

} // namespace KTp

#include "moc_telepathy-handler-application.cpp"
