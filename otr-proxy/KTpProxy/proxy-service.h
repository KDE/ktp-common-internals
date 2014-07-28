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

#ifndef KTP_PROXY_PROXY_SERVICE_HEADER
#define KTP_PROXY_PROXY_SERVICE_HEADER

#include "proxy-service-adaptee.h"
#include "proxy-observer.h"
#include "types.h"
#include "otr-manager.h"

#include <TelepathyQt/AbstractClientObserver>
#include <TelepathyQt/Types>
#include <TelepathyQt/DBusService>

#include <QDBusConnection>
#include <QMap>

namespace Tp
{
    class PendingOperation;
}


class ProxyService : public Tp::DBusService
{
    Q_OBJECT
    Q_DISABLE_COPY(ProxyService)

    public:
        ProxyService(const QDBusConnection &dbusConnection, OTR::Config *config);
        ~ProxyService();

        void addChannel(const Tp::ChannelPtr &channel, const Tp::AccountPtr &account);

        void registerService(Tp::DBusError *error);

        QVariantMap immutableProperties() const;

    private Q_SLOTS:
        void onChannelProxyClosed();

    private Q_SLOTS:
        void onChannelReady(Tp::PendingOperation *pendingChanReady);

    private:
        ProxyServiceAdaptee adaptee;
        QMap<OtrProxyChannel*, OtrProxyChannelPtr> channels;
        ProxyObserverPtr observer;
        Tp::ClientRegistrarPtr registrar;
        OTR::Manager manager;
};

#endif
