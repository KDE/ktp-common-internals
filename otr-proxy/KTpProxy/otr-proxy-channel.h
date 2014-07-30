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

#ifndef KTP_PROXY_OTR_PROXY_CHANNEL_HEADER
#define KTP_PROXY_OTR_PROXY_CHANNEL_HEADER

#include "types.h"
#include "otr-utils.h"

#include <TelepathyQt/DBusService>
#include <TelepathyQt/TextChannel>

#include <QDBusObjectPath>

class ProxyService;

namespace OTR
{
    class Manager;
}

class OtrProxyChannel : public Tp::DBusService
{

    Q_OBJECT
    Q_DISABLE_COPY(OtrProxyChannel)

    private:
        OtrProxyChannel(const QDBusConnection &dbusConnection,
                const Tp::TextChannelPtr &channel,
                const OTR::SessionContext &context,
                ProxyService *ps);

    public:
        static OtrProxyChannelPtr create(const QDBusConnection &dbusConnection, const Tp::TextChannelPtr &channel,
                const OTR::SessionContext &context, ProxyService *ps);

        void registerService(Tp::DBusError *error);

        QVariantMap immutableProperties() const;

        bool isConnected() const;

        Tp::TextChannelPtr wrappedChannel() const;

        class Adaptee;

    Q_SIGNALS:
        void connected(const QDBusObjectPath &proxyPath);
        void disconnected(const QDBusObjectPath &proxyPath);
        void closed();

    private Q_SLOTS:
        void onClosed();

    private:
        Adaptee *adaptee;
};

#endif /* KTP_PROXY_OTR_PROXY_CHANNEL_HEADER */
