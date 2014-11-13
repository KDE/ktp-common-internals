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
#ifndef SERVICE_AVAILABILITY_CHECKER_H
#define SERVICE_AVAILABILITY_CHECKER_H

#include <QtCore/QObject>

#include <KTp/ktpcommoninternals_export.h>

class QDBusPendingCallWatcher;

namespace KTp
{

/**
 * This class watches if a given d-bus service is either
 * available on the bus or can be activated on demand.
 */
class KTPCOMMONINTERNALS_EXPORT ServiceAvailabilityChecker : public QObject
{
    Q_OBJECT
public:
    explicit ServiceAvailabilityChecker(const QString & serviceName, QObject *parent = 0);
    virtual ~ServiceAvailabilityChecker();

    bool isAvailable() const;

private Q_SLOTS:
    void introspect();
    void onCallFinished(QDBusPendingCallWatcher *watcher);
    void onServiceOwnerChanged(const QString & service,
                               const QString & oldOwner,
                               const QString & newOwner);

private:
    struct Private;
    Private * const d;
};

}

#endif // SERVICE_AVAILABILITY_CHECKER_H
