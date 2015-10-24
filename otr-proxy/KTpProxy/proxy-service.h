/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public		   *
 * License as published by the Free Software Foundation; either		   *
 * version 2.1 of the License, or (at your option) any later version.	   *
 * 									   *
 * This library is distributed in the hope that it will be useful,	   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU	   *
 * Lesser General Public License for more details.			   *
 * 									   *
 * You should have received a copy of the GNU Lesser General Public	   *
 * License along with this library; if not, write to the Free Software	   *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA*
 ***************************************************************************/

#ifndef KTP_PROXY_PROXY_SERVICE_HEADER
#define KTP_PROXY_PROXY_SERVICE_HEADER

#include "proxy-service-adaptee.h"
#include "proxy-observer.h"
#include "otr-manager.h"

#include "KTp/OTR/types.h"

#include <TelepathyQt/AbstractClientObserver>
#include <TelepathyQt/Types>
#include <TelepathyQt/DBusService>
#include <TelepathyQt/AccountManager>

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
        ProxyService(const QDBusConnection &dbusConnection, OTR::Config *config, const Tp::ClientRegistrarPtr &registrar);
        ~ProxyService();

        void addChannel(const Tp::TextChannelPtr &channel, const Tp::AccountPtr &account);

        void registerService(Tp::DBusError *error);

        QVariantMap immutableProperties() const;
        OtrlPolicy getPolicy() const;
        void setPolicy(OtrlPolicy otrPolicy);

        /** returns false if key cannot be generated - i.e. incorrect id */
        bool createNewPrivateKey(const QString &accountId, const QString &accountName);
        QString getFingerprintFor(const QString &accountId, const QString &accountName);
        KTp::FingerprintInfoList getKnownFingerprints(const QString &accountId);
        bool trustFingerprint(const QString &accountId, const QString &contactName, const QString &fingerprint, bool trust);
        bool forgetFingerprint(const QString &accountId, const QString &contactName, const QString &fingerprint);

        OTR::Manager* managerOTR();
        Tp::AccountManagerPtr accountManager();

    private Q_SLOTS:
        void onChannelProxyClosed();
        void onKeyGenerationThreadFinished();

    Q_SIGNALS:
        void keyGenerationStarted(const QString &accountId);
        void keyGenerationFinished(const QString &accountId, bool error);

    private:
        ProxyServiceAdaptee adaptee;
        QMap<OtrProxyChannel*, OtrProxyChannelPtr> channels;
        ProxyObserverPtr observer;
        Tp::ClientRegistrarPtr registrar;
        OTR::Manager manager;
        Tp::AccountManagerPtr am;
};

#endif
