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

#include "types.h"

#include <QDBusArgument>
#include <QDBusMetaType>

namespace Tp
{

TP_QT_EXPORT bool operator==(const FingerprintInfo& v1, const FingerprintInfo& v2)
{
    return ((v1.contactName == v2.contactName)
            && (v1.fingerprint == v2.fingerprint)
            && (v1.isVerified == v2.isVerified)
            );
}

TP_QT_EXPORT QDBusArgument& operator<<(QDBusArgument& arg, const FingerprintInfo& val)
{
    arg.beginStructure();
    arg << val.contactName << val.fingerprint << val.isVerified;
    arg.endStructure();
    return arg;
}

TP_QT_EXPORT const QDBusArgument& operator>>(const QDBusArgument& arg, FingerprintInfo& val)
{
    arg.beginStructure();
    arg >> val.contactName >> val.fingerprint >> val.isVerified;
    arg.endStructure();
    return arg;
}

void registerProxyTypes()
{
    static bool registered = false;
    if(registered) {
        return;
    }
    registered = true;

    qDBusRegisterMetaType<Tp::FingerprintInfo>();
    qDBusRegisterMetaType<Tp::FingerprintInfoList>();
}

}
