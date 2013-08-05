/*
  * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef KTP_ABSTRACTLOGGERPLUGIN_H
#define KTP_ABSTRACTLOGGERPLUGIN_H

#include <QtCore/QObject>
#include <TelepathyQt/Types>
#include <KTp/message.h>

namespace KTp {

class PendingLoggerDates;
class PendingLoggerLogs;

class AbstractLoggerPlugin : public QObject
{
    Q_OBJECT

  public:
    explicit AbstractLoggerPlugin();
    virtual ~AbstractLoggerPlugin();

    virtual KTp::PendingLoggerDates* queryDates(const Tp::AccountPtr &account,
                                                const Tp::ContactPtr &contact) = 0;

    virtual KTp::PendingLoggerLogs* queryLogs(const Tp::AccountPtr &account,
                                              const Tp::ContactPtr &contact,
                                              const QDate &date) = 0;

};

}

#endif // KTP_ABSTRACTLOGGERPLUGIN_H
