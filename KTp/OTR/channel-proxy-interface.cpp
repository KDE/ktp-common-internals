#define IN_TP_QT_HEADER
#include "channel-proxy-interface.h"

namespace KTp
{
namespace Client
{

ChannelProxyInterfaceOTRInterface::ChannelProxyInterfaceOTRInterface(const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
}

ChannelProxyInterfaceOTRInterface::ChannelProxyInterfaceOTRInterface(const QDBusConnection& connection, const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), connection, parent)
{
}

ChannelProxyInterfaceOTRInterface::ChannelProxyInterfaceOTRInterface(Tp::DBusProxy *proxy)
    : Tp::AbstractInterface(proxy, staticInterfaceName())
{
}

ChannelProxyInterfaceOTRInterface::ChannelProxyInterfaceOTRInterface(const Tp::AbstractInterface& mainInterface)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), mainInterface.parent())
{
}

ChannelProxyInterfaceOTRInterface::ChannelProxyInterfaceOTRInterface(const Tp::AbstractInterface& mainInterface, QObject *parent)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), parent)
{
}

void ChannelProxyInterfaceOTRInterface::invalidate(Tp::DBusProxy *proxy,
        const QString &error, const QString &message)
{
    disconnect(this, SIGNAL(MessageSent(const Tp::MessagePartList&, uint, const QString&)), nullptr, nullptr);
    disconnect(this, SIGNAL(MessageReceived(const Tp::MessagePartList&)), nullptr, nullptr);
    disconnect(this, SIGNAL(PendingMessagesRemoved(const Tp::UIntList&)), nullptr, nullptr);
    disconnect(this, SIGNAL(PeerAuthenticationRequested(const QString&)), nullptr, nullptr);
    disconnect(this, SIGNAL(PeerAuthenticationConcluded(bool)), nullptr, nullptr);
    disconnect(this, SIGNAL(PeerAuthenticationInProgress()), nullptr, nullptr);
    disconnect(this, SIGNAL(PeerAuthenticationAborted()), nullptr, nullptr);
    disconnect(this, SIGNAL(PeerAuthenticationError()), nullptr, nullptr);
    disconnect(this, SIGNAL(PeerAuthenticationCheated()), nullptr, nullptr);
    disconnect(this, SIGNAL(SessionRefreshed()), nullptr, nullptr);
    disconnect(this, SIGNAL(TrustLevelChanged(uint)), nullptr, nullptr);

    Tp::AbstractInterface::invalidate(proxy, error, message);
}
}
}
