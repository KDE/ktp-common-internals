/*
    Copyright (C) 2011 Collabora Ltd. <info@collabora.com>
      @author George Kiagiadakis <george.kiagiadakis@collabora.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "service-availability-checker.h"

#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusServiceWatcher>
#include <QtDBus/QDBusPendingCall>
#include <QtDBus/QDBusReply>

#include <KDebug>

namespace KTp
{

struct ServiceAvailabilityChecker::Private
{
    QString serviceName;
    bool serviceAvailable;
    bool serviceActivatable;
};

ServiceAvailabilityChecker::ServiceAvailabilityChecker(const QString & serviceName, QObject *parent)
    : QObject(parent), d(new Private)
{
    d->serviceName = serviceName;
    d->serviceAvailable = false;
    d->serviceActivatable = false;

    QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(serviceName,
            QDBusConnection::sessionBus(),
            QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
            this);
    connect(serviceWatcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this, SLOT(onServiceOwnerChanged(QString,QString,QString)));

    introspect();
}

ServiceAvailabilityChecker::~ServiceAvailabilityChecker()
{
    delete d;
}

bool ServiceAvailabilityChecker::isAvailable() const
{
    return d->serviceAvailable || d->serviceActivatable;
}

void ServiceAvailabilityChecker::introspect()
{
    QDBusConnectionInterface *dbusIface = QDBusConnection::sessionBus().interface();

    QDBusPendingCall call = dbusIface->asyncCall(QLatin1String("ListActivatableNames"));
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(onCallFinished(QDBusPendingCallWatcher*)));
    watcher->setObjectName(QLatin1String("ListActivatableNamesWatcher"));

    call = dbusIface->asyncCall(QLatin1String("ListNames"));
    watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(onCallFinished(QDBusPendingCallWatcher*)));
}

void ServiceAvailabilityChecker::onCallFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusReply<QStringList> reply = *watcher;
    if (!reply.isValid()) {
        kDebug() << "Got error while introspecting service availability:" << reply.error();
    } else {
        if (watcher->objectName() == QLatin1String("ListActivatableNamesWatcher")) {
            d->serviceActivatable = reply.value().contains(d->serviceName);
        } else {
            if (!d->serviceAvailable) {
                d->serviceAvailable = reply.value().contains(d->serviceName);
            }
            //else onServiceOwnerChanged() has been emitted before the introspection finished
            //so the reply we got here may be incorrect, claiming that the service is not available
        }
    }

    watcher->deleteLater();
}

void ServiceAvailabilityChecker::onServiceOwnerChanged(const QString & service,
        const QString & oldOwner, const QString & newOwner)
{
    Q_UNUSED(oldOwner);

    if (service == d->serviceName) {
        d->serviceAvailable = !newOwner.isEmpty();
    }
}

}

#include "service-availability-checker.moc"
