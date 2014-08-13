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

#ifndef KTP_PROXY_TYPES_HEADER
#define KTP_PROXY_TYPES_HEADER

#include <TelepathyQt/SharedPtr>
#include <QDBusArgument>

class OtrProxyChannel;
class ProxyService;
class ProxyObserver;

typedef Tp::SharedPtr<OtrProxyChannel> OtrProxyChannelPtr;
typedef Tp::SharedPtr<ProxyService> ProxyServicePtr;
typedef Tp::SharedPtr<ProxyObserver> ProxyObserverPtr;

namespace Tp
{

/**
 * \struct FingerprintInfo
 * \ingroup struct
 * \headerfile TelepathyQt/types.h <TelepathyQt/Types>
 *
 * Structure type generated from the specification.
 *
 * A struct (Contact_Name, Fingerprint, Is_Verified) representing remote
 * contact&apos;s fingerprint, as returned by Get_Known_Fingerprints
 */
struct TP_QT_EXPORT FingerprintInfo
{
    QString contactName;
    QString fingerprint;
    bool isVerified;
    bool inUse;
};

TP_QT_EXPORT bool operator==(const FingerprintInfo& v1, const FingerprintInfo& v2);
inline bool operator!=(const FingerprintInfo& v1, const FingerprintInfo& v2)
{
    return !operator==(v1, v2);
}
TP_QT_EXPORT QDBusArgument& operator<<(QDBusArgument& arg, const FingerprintInfo& val);
TP_QT_EXPORT const QDBusArgument& operator>>(const QDBusArgument& arg, FingerprintInfo& val);

/**
 * \ingroup list
 * \headerfile TelepathyQt/types.h <TelepathyQt/Types>
 *
 * Array of FingerprintInfo values.
 */
typedef QList<FingerprintInfo> FingerprintInfoList;

void registerProxyTypes();

}

Q_DECLARE_METATYPE(Tp::FingerprintInfo)
Q_DECLARE_METATYPE(Tp::FingerprintInfoList)

#endif
