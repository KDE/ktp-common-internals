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

#include <QTimer>
#include <KCmdLineArgs>
#include <KDebug>

#include <TelepathyQt/Types>
#include <TelepathyQt/Debug>


extern bool kde_kdebug_enable_dbus_interface;

namespace KTp
{

namespace {
int s_tpqt4DebugArea;

static void tpDebugCallback(const QString &libraryName,
                            const QString &libraryVersion,
                            QtMsgType type,
                            const QString &msg)
{
    Q_UNUSED(libraryName)
    Q_UNUSED(libraryVersion)
    kDebugStream(type, s_tpqt4DebugArea, __FILE__, __LINE__, 0) << qPrintable(msg);
}
}

class TelepathyHandlerApplication::Private
{
public:
    Private(TelepathyHandlerApplication *q);
    ~Private();

    void _k_onInitialTimeout();
    void _k_onTimeout();

    static KComponentData initHack();
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
    if (jobCount == 0 && jobCount.fetchAndAddOrdered(-1) == 0) {
        // m_jobCount is now -1
        kDebug() << "No job received. Exiting";
        QCoreApplication::quit();
    }
}

void TelepathyHandlerApplication::Private::_k_onTimeout()
{
    if (jobCount == 0 && jobCount.fetchAndAddOrdered(-1) == 0) {
        // m_jobCount is now -1
        kDebug() << "Timeout. Exiting";
        QCoreApplication::quit();
    }
}

// this gets called before even entering QApplication::QApplication()
KComponentData TelepathyHandlerApplication::Private::initHack()
{
    KComponentData cData(KCmdLineArgs::aboutData());
    KCmdLineOptions handler_options;
    handler_options.add("persist", ki18n("Persistent mode (do not exit on timeout)"));
    handler_options.add("debug", ki18n("Show Telepathy debugging information"));
    KCmdLineArgs::addCmdLineOptions(handler_options, ki18n("KDE Telepathy"), "kde-telepathy", "kde");
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde-telepathy");
    Private::s_persist = args->isSet("persist");
    Private::s_debug = args->isSet("debug");

    s_tpqt4DebugArea = KDebug::registerArea("Telepathy-Qt4");

    return cData;
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

    // Redirect Tp debug and warnings to KDebug output
    Tp::setDebugCallback(&tpDebugCallback);

    // Enable telepathy-Qt4 debug
    Tp::enableDebug(s_debug);
    Tp::enableWarnings(true);

    // Enable KDebug DBus interface
    // FIXME This must be enabled here because there is a bug in plasma
    //       it should be removed when this is fixed
    kde_kdebug_enable_dbus_interface = s_debug;

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


TelepathyHandlerApplication::TelepathyHandlerApplication(bool GUIenabled,
                                                         int initialTimeout,
                                                         int timeout)
    : KApplication(GUIenabled, Private::initHack()),
      d(new Private(this))
{
    d->init(initialTimeout, timeout);
}

TelepathyHandlerApplication::TelepathyHandlerApplication(Display *display,
                                                         Qt::HANDLE visual,
                                                         Qt::HANDLE colormap,
                                                         int initialTimeout,
                                                         int timeout)
    : KApplication(display, visual, colormap, Private::initHack()),
      d(new Private(this))
{
    d->init(initialTimeout, timeout);
}

TelepathyHandlerApplication::~TelepathyHandlerApplication()
{
    delete d;
}

int TelepathyHandlerApplication::newJob()
{
    TelepathyHandlerApplication *app = qobject_cast<TelepathyHandlerApplication*>(KApplication::kApplication());
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
    return ret;
}

void TelepathyHandlerApplication::jobFinished()
{
    TelepathyHandlerApplication *app = qobject_cast<TelepathyHandlerApplication*>(KApplication::kApplication());
    TelepathyHandlerApplication::Private *d = app->d;

    if (d->jobCount.fetchAndAddOrdered(-1) <= 1) {
        if (!Private::s_persist && d->timeout >= 0) {
            kDebug() << "No other jobs at the moment. Starting timer.";
            d->timer->start(d->timeout);
        }
    }
}

} // namespace KTp

#include "telepathy-handler-application.moc"
