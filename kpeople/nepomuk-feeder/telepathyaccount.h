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

#ifndef TELEPATHY_INTEGRATION_DAEMON_TELEPATHYACCOUNT_H
#define TELEPATHY_INTEGRATION_DAEMON_TELEPATHYACCOUNT_H

#include <QtCore/QObject>
#include <QtCore/QString>

class TelepathyAccount : public QObject
{
    Q_OBJECT

public:
    TelepathyAccount(const QString &path, QObject *parent = 0);
    ~TelepathyAccount();

private Q_SLOTS:

private:
    QString m_path;
};


#endif // Header guard

