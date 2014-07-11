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
    connect(adaptee, SIGNAL(proxyconnected(const QDBusObjectPath&)), SIGNAL(ProxyConnected(const QDBusObjectPath&)));
    connect(adaptee, SIGNAL(proxydisconnected(const QDBusObjectPath&)), SIGNAL(ProxyDisconnected(const QDBusObjectPath&)));
}

ProxyServiceAdaptor::~ProxyServiceAdaptor()
{
}

}
}
