#include "svc-proxy-service.h"

#include <TelepathyQt/Constants>
#include <TelepathyQt/MethodInvocationContext>

namespace Tp
{
namespace Service
{

ProxyServiceAdaptor::ProxyServiceAdaptor(const QDBusConnection& bus, QObject* adaptee, QObject* parent)
    : Tp::AbstractAdaptor(bus, adaptee, parent)
{
    connect(adaptee, SIGNAL(proxyConnected(const QDBusObjectPath&)), SIGNAL(ProxyConnected(const QDBusObjectPath&)));
    connect(adaptee, SIGNAL(proxyDisconnected(const QDBusObjectPath&)), SIGNAL(ProxyDisconnected(const QDBusObjectPath&)));
    connect(adaptee, SIGNAL(keyGenerationStarted(const QDBusObjectPath&)), SIGNAL(KeyGenerationStarted(const QDBusObjectPath&)));
    connect(adaptee, SIGNAL(keyGenerationFinished(const QDBusObjectPath&, bool)), SIGNAL(KeyGenerationFinished(const QDBusObjectPath&, bool)));
}

ProxyServiceAdaptor::~ProxyServiceAdaptor()
{
}

uint ProxyServiceAdaptor::PolicySettings() const
{
    return qvariant_cast< uint >(adaptee()->property("policySettings"));
}

void ProxyServiceAdaptor::SetPolicySettings(const uint &newValue)
{
    adaptee()->setProperty("policySettings", qVariantFromValue(newValue));
}

void ProxyServiceAdaptor::GeneratePrivateKey(const QDBusObjectPath& account, const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("generatePrivateKey(QDBusObjectPath,Tp::Service::ProxyServiceAdaptor::GeneratePrivateKeyContextPtr)") == -1) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return;
    }

    GeneratePrivateKeyContextPtr ctx = GeneratePrivateKeyContextPtr(
            new Tp::MethodInvocationContext<  >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "generatePrivateKey",
        Q_ARG(QDBusObjectPath, account),
        Q_ARG(Tp::Service::ProxyServiceAdaptor::GeneratePrivateKeyContextPtr, ctx));
}

QString ProxyServiceAdaptor::GetFingerprintForAccount(const QDBusObjectPath& account, const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("getFingerprintForAccount(QDBusObjectPath,Tp::Service::ProxyServiceAdaptor::GetFingerprintForAccountContextPtr)") == -1) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return QString();
    }

    GetFingerprintForAccountContextPtr ctx = GetFingerprintForAccountContextPtr(
            new Tp::MethodInvocationContext< QString >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "getFingerprintForAccount",
        Q_ARG(QDBusObjectPath, account),
        Q_ARG(Tp::Service::ProxyServiceAdaptor::GetFingerprintForAccountContextPtr, ctx));
    return QString();
}

KTp::FingerprintInfoList ProxyServiceAdaptor::GetKnownFingerprints(const QDBusObjectPath& account, const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("getKnownFingerprints(QDBusObjectPath,Tp::Service::ProxyServiceAdaptor::GetKnownFingerprintsContextPtr)") == -1) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return KTp::FingerprintInfoList();
    }

    GetKnownFingerprintsContextPtr ctx = GetKnownFingerprintsContextPtr(
            new Tp::MethodInvocationContext< KTp::FingerprintInfoList >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "getKnownFingerprints",
        Q_ARG(QDBusObjectPath, account),
        Q_ARG(Tp::Service::ProxyServiceAdaptor::GetKnownFingerprintsContextPtr, ctx));
    return KTp::FingerprintInfoList();
}

void ProxyServiceAdaptor::TrustFingerprint(const QDBusObjectPath& account, const QString& contactName, const QString& fingerprint, bool trust, const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("trustFingerprint(QDBusObjectPath,QString,QString,bool,Tp::Service::ProxyServiceAdaptor::TrustFingerprintContextPtr)") == -1) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return;
    }

    TrustFingerprintContextPtr ctx = TrustFingerprintContextPtr(
            new Tp::MethodInvocationContext<  >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "trustFingerprint",
        Q_ARG(QDBusObjectPath, account), Q_ARG(QString, contactName), Q_ARG(QString, fingerprint), Q_ARG(bool, trust),
        Q_ARG(Tp::Service::ProxyServiceAdaptor::TrustFingerprintContextPtr, ctx));
}

void ProxyServiceAdaptor::ForgetFingerprint(const QDBusObjectPath& account, const QString& contactName, const QString& fingerprint, const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("forgetFingerprint(QDBusObjectPath,QString,QString,Tp::Service::ProxyServiceAdaptor::ForgetFingerprintContextPtr)") == -1) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return;
    }

    ForgetFingerprintContextPtr ctx = ForgetFingerprintContextPtr(
            new Tp::MethodInvocationContext<  >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "forgetFingerprint",
        Q_ARG(QDBusObjectPath, account), Q_ARG(QString, contactName), Q_ARG(QString, fingerprint),
        Q_ARG(Tp::Service::ProxyServiceAdaptor::ForgetFingerprintContextPtr, ctx));
}

}
}
