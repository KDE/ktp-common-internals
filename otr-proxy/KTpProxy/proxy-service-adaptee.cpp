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
            return;
        case 1:
            ps->setPolicy(OTRL_POLICY_OPPORTUNISTIC);
            return;
        case 2:
            ps->setPolicy(OTRL_POLICY_MANUAL);
            return;
        case 3:
            ps->setPolicy(OTRL_POLICY_NEVER);
            return;
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
    if(ac && ac->isValid() &&
            ps->createNewPrivateKey(OTR::utils::accountIdFor(accountPath), ac->displayName())) {
        context->setFinished();
    } else {
        // TODO better errors
        context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT,
                QLatin1String("Could not generate private key for given account"));
    }
}

void ProxyServiceAdaptee::getFingerprintForAccount(QDBusObjectPath accountPath,
        const Tp::Service::ProxyServiceAdaptor::GetFingerprintForAccountContextPtr &context)
{
    Tp::AccountPtr ac = ps->accountManager()->accountForObjectPath(accountPath.path());
    if(ac && ac->isValid()) {
        context->setFinished(ps->getFingerprintFor(OTR::utils::accountIdFor(accountPath), ac->displayName()));
    } else {
        context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT, QLatin1String("No such account"));
    }
}

void ProxyServiceAdaptee::getKnownFingerprints(const QDBusObjectPath &accountPath,
        const Tp::Service::ProxyServiceAdaptor::GetKnownFingerprintsContextPtr &context)
{
    Tp::AccountPtr ac = ps->accountManager()->accountForObjectPath(accountPath.path());
    if(ac && ac->isValid()) {
        context->setFinished(ps->getKnownFingerprints(OTR::utils::accountIdFor(accountPath)));
    } else {
        context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT, QLatin1String("No such account"));
    }
}

void ProxyServiceAdaptee::trustFingerprint(const QDBusObjectPath &accountPath, const QString &contactName,
        const QString &fingerprint, bool trust, const Tp::Service::ProxyServiceAdaptor::TrustFingerprintContextPtr &context)
{
    Tp::AccountPtr ac = ps->accountManager()->accountForObjectPath(accountPath.path());
    if(ac && ac->isValid()) {
        if(ps->trustFingerprint(OTR::utils::accountIdFor(accountPath), contactName, fingerprint, trust)) {
            context->setFinished();
        } else {
            context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT, QLatin1String("Invalid finerprint info"));
        }
    } else {
        context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT, QLatin1String("No such account"));
    }
}

void ProxyServiceAdaptee::forgetFingerprint(const QDBusObjectPath &accountPath, const QString &contactName,
        const QString &fingerprint, const Tp::Service::ProxyServiceAdaptor::ForgetFingerprintContextPtr &context)
{
    Tp::AccountPtr ac = ps->accountManager()->accountForObjectPath(accountPath.path());
    if(ac && ac->isValid()) {
        if(ps->forgetFingerprint(OTR::utils::accountIdFor(accountPath), contactName, fingerprint)) {
            context->setFinished();
        } else {
            context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT,
                   QLatin1String("Invalid finerprint info or fingerpirnt is in use"));
        }
    } else {
        context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT, QLatin1String("No such account"));
    }
}
