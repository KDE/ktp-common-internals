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

#include "proxy-service-adaptee.h"
#include "proxy-service.h"
#include "otr-constants.h"

#include <TelepathyQt/DBusObject>

ProxyServiceAdaptee::ProxyServiceAdaptee(ProxyService *ps, const QDBusConnection &dbusConnection)
    : adaptor(new Tp::Service::ProxyServiceAdaptor(dbusConnection, this, ps->dbusObject())),
    ps(ps)
{
    connect(ps, SIGNAL(keyGenerationStarted(const QString&)), SLOT(onKeyGenerationStarted(const QString&)));
    connect(ps, SIGNAL(keyGenerationFinished(const QString&, bool)), SLOT(onKeyGenerationFinished(const QString&, bool)));
}

ProxyServiceAdaptee::~ProxyServiceAdaptee() { }

uint ProxyServiceAdaptee::policy() const
{
    switch(ps->getPolicy()) {
        case OTRL_POLICY_ALWAYS:
            return 0;
        case OTRL_POLICY_OPPORTUNISTIC:
            return 1;
        case OTRL_POLICY_MANUAL:
            return 2;
        case OTRL_POLICY_NEVER:
        default:
            return 3;
    }
}

void ProxyServiceAdaptee::setPolicy(uint otrPolicy)
{
    switch(otrPolicy) {
        case 0:
            ps->setPolicy(OTRL_POLICY_ALWAYS);
        case 1:
            ps->setPolicy(OTRL_POLICY_OPPORTUNISTIC);
        case 2:
            ps->setPolicy(OTRL_POLICY_MANUAL);
        case 3:
            ps->setPolicy(OTRL_POLICY_NEVER);
        default:
            // nos uch policy
            return;
    }
}

void ProxyServiceAdaptee::onProxyConnected(const QDBusObjectPath &proxyPath)
{
    Q_EMIT proxyConnected(proxyPath);
}
void ProxyServiceAdaptee::onProxyDisconnected(const QDBusObjectPath &proxyPath)
{
    Q_EMIT proxyDisconnected(proxyPath);
}

void ProxyServiceAdaptee::onKeyGenerationStarted(const QString &accountId)
{
    Q_EMIT(keyGenerationStarted(OTR::utils::objectPathFor(accountId)));
}

void ProxyServiceAdaptee::onKeyGenerationFinished(const QString &accountId, bool error)
{
    Q_EMIT(keyGenerationFinished(OTR::utils::objectPathFor(accountId), error));
}

void ProxyServiceAdaptee::generatePrivateKey(const QDBusObjectPath &accountPath,
        const Tp::Service::ProxyServiceAdaptor::GeneratePrivateKeyContextPtr &context)
{
    Tp::AccountPtr ac = ps->accountManager()->accountForObjectPath(accountPath.path());
    if(ac->isValidAccount() &&
            ps->createNewPrivateKey(OTR::utils::accountIdFor(accountPath), ac->normalizedName())) {
        context->setFinished();
    } else {
        // TODO better errors
        context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT,
                QLatin1String("Could not generate private key for given account"));
    }
}
