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

}
}
