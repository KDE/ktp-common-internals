/*
 * This file contains D-Bus client proxy classes generated by qt-client-gen.py.
 *
 * This file can be distributed under the same terms as the specification from
 * which it was generated.
 */

#ifndef OTR_PROXY_SERVICE_INTERFACE_HEADER
#define OTR_PROXY_SERVICE_INTERFACE_HEADER

#include "types.h"

#include "ktpotr_export.h"

#include <TelepathyQt/Types>

#include <QtGlobal>

#include <QString>
#include <QObject>
#include <QVariant>

#include <QDBusPendingReply>

#include <TelepathyQt/AbstractInterface>
#include <TelepathyQt/DBusProxy>
#include <TelepathyQt/Global>


namespace Tp
{
class PendingVariant;
class PendingOperation;
}

namespace KTp
{
namespace Client
{

/**
 * \class ProxyServiceInterface
 * \headerfile TelepathyQt/proxy-service.h <TelepathyQt/ProxyService>
 * \ingroup clientproxyservice
 *
 * Proxy class providing a 1:1 mapping of the D-Bus interface "org.kde.TelepathyProxy.ProxyService".
 */
class KTPOTR_EXPORT ProxyServiceInterface : public Tp::AbstractInterface
{
    Q_OBJECT

public:
    /**
     * Returns the name of the interface "org.kde.TelepathyProxy.ProxyService", which this class
     * represents.
     *
     * \return The D-Bus interface name.
     */
    static inline QLatin1String staticInterfaceName()
    {
        return QLatin1String("org.kde.TelepathyProxy.ProxyService");
    }

    /**
     * Creates a ProxyServiceInterface associated with the given object on the session bus.
     *
     * \param busName Name of the service the object is on.
     * \param objectPath Path to the object on the service.
     * \param parent Passed to the parent class constructor.
     */
    ProxyServiceInterface(
        const QString& busName,
        const QString& objectPath,
        QObject* parent = nullptr
    );

    /**
     * Creates a ProxyServiceInterface associated with the given object on the given bus.
     *
     * \param connection The bus via which the object can be reached.
     * \param busName Name of the service the object is on.
     * \param objectPath Path to the object on the service.
     * \param parent Passed to the parent class constructor.
     */
    ProxyServiceInterface(
        const QDBusConnection& connection,
        const QString& busName,
        const QString& objectPath,
        QObject* parent = nullptr
    );

    /**
     * Creates a ProxyServiceInterface associated with the same object as the given proxy.
     *
     * \param proxy The proxy to use. It will also be the QObject::parent()
     *               for this object.
     */
    ProxyServiceInterface(Tp::DBusProxy *proxy);

    /**
     * Creates a ProxyServiceInterface associated with the same object as the given proxy.
     * Additionally, the created proxy will have the same parent as the given
     * proxy.
     *
     * \param mainInterface The proxy to use.
     */
    explicit ProxyServiceInterface(const Tp::AbstractInterface& mainInterface);

    /**
     * Creates a ProxyServiceInterface associated with the same object as the given proxy.
     * However, a different parent object can be specified.
     *
     * \param mainInterface The proxy to use.
     * \param parent Passed to the parent class constructor.
     */
    ProxyServiceInterface(const Tp::AbstractInterface& mainInterface, QObject* parent);

    /**
     * Asynchronous getter for the remote object property \c PolicySettings of type \c uint.
     *
     *
     * \htmlonly
     * <p>Set the OTR policy how you like it</p>
     * \endhtmlonly
     *
     * \return A pending variant which will emit finished when the property has been
     *          retrieved.
     */
    inline Tp::PendingVariant *requestPropertyPolicySettings() const
    {
        return internalRequestProperty(QLatin1String("PolicySettings"));
    }

    /**
     * Asynchronous setter for the remote object property \c PolicySettings of type \c uint.
     *
     *
     * \htmlonly
     * <p>Set the OTR policy how you like it</p>
     * \endhtmlonly
     *
     * \return A pending operation which will emit finished when the property has been
     *          set.
     */
    inline Tp::PendingOperation *setPropertyPolicySettings(uint newValue)
    {
        return internalSetProperty(QLatin1String("PolicySettings"), QVariant::fromValue(newValue));
    }

    /**
     * Request all of the DBus properties on the interface.
     *
     * \return A pending variant map which will emit finished when the properties have
     *          been retrieved.
     */
    Tp::PendingVariantMap *requestAllProperties() const
    {
        return internalRequestAllProperties();
    }

public Q_SLOTS:
    /**
     * Begins a call to the D-Bus method \c GeneratePrivateKey on the remote object.
     *
     * \htmlonly
     * <p> Generate new private key for given account. </p>
     * \endhtmlonly
     *
     * Note that \a timeout is ignored as of now. It will be used once
     * http://bugreports.qt.nokia.com/browse/QTBUG-11775 is fixed.
     *
     * \param timeout The timeout in milliseconds.
     */
    inline QDBusPendingReply<> GeneratePrivateKey(const QDBusObjectPath& account, int timeout = -1)
    {
        if (!invalidationReason().isEmpty()) {
            return QDBusPendingReply<>(QDBusMessage::createError(
                invalidationReason(),
                invalidationMessage()
            ));
        }

        QDBusMessage callMessage = QDBusMessage::createMethodCall(this->service(), this->path(),
                this->staticInterfaceName(), QLatin1String("GeneratePrivateKey"));
        callMessage << QVariant::fromValue(account);
        return this->connection().asyncCall(callMessage, timeout);
    }

    /**
     * Begins a call to the D-Bus method \c GetFingerprintForAccount on the remote object.
     *
     * Get private key fingerprint associated with given account
     *
     * Note that \a timeout is ignored as of now. It will be used once
     * http://bugreports.qt.nokia.com/browse/QTBUG-11775 is fixed.
     *
     *
     * \param account
     *
     *     The account the new key is generated for
     * \param timeout The timeout in milliseconds.
     *
     * \return
     *
     *     Fingerprint of given account&apos;s private key or an empty string
     *     if none exists
     */
    inline QDBusPendingReply<QString> GetFingerprintForAccount(const QDBusObjectPath& account, int timeout = -1)
    {
        if (!invalidationReason().isEmpty()) {
            return QDBusPendingReply<QString>(QDBusMessage::createError(
                invalidationReason(),
                invalidationMessage()
            ));
        }

        QDBusMessage callMessage = QDBusMessage::createMethodCall(this->service(), this->path(),
                this->staticInterfaceName(), QLatin1String("GetFingerprintForAccount"));
        callMessage << QVariant::fromValue(account);
        return this->connection().asyncCall(callMessage, timeout);
    }

    /**
     * Begins a call to the D-Bus method \c GetKnownFingerprints on the remote object.
     *
     * Get private key fingerprint associated with given account
     *
     * Note that \a timeout is ignored as of now. It will be used once
     * http://bugreports.qt.nokia.com/browse/QTBUG-11775 is fixed.
     *
     * \param timeout The timeout in milliseconds.
     */
    inline QDBusPendingReply<KTp::FingerprintInfoList> GetKnownFingerprints(const QDBusObjectPath& account, int timeout = -1)
    {
        if (!invalidationReason().isEmpty()) {
            return QDBusPendingReply<KTp::FingerprintInfoList>(QDBusMessage::createError(
                invalidationReason(),
                invalidationMessage()
            ));
        }

        QDBusMessage callMessage = QDBusMessage::createMethodCall(this->service(), this->path(),
                this->staticInterfaceName(), QLatin1String("GetKnownFingerprints"));
        callMessage << QVariant::fromValue(account);
        return this->connection().asyncCall(callMessage, timeout);
    }

    /**
     * Begins a call to the D-Bus method \c TrustFingerprint on the remote object.
     *
     * Trust or distrust given fingerprint for account by settings
     * Is_Verfified to %TRUE or %FALSE
     *
     * Note that \a timeout is ignored as of now. It will be used once
     * http://bugreports.qt.nokia.com/browse/QTBUG-11775 is fixed.
     *
     * \param timeout The timeout in milliseconds.
     */
    inline QDBusPendingReply<> TrustFingerprint(const QDBusObjectPath& account, const QString& contactName, const QString& fingerprint, bool trust, int timeout = -1)
    {
        if (!invalidationReason().isEmpty()) {
            return QDBusPendingReply<>(QDBusMessage::createError(
                invalidationReason(),
                invalidationMessage()
            ));
        }

        QDBusMessage callMessage = QDBusMessage::createMethodCall(this->service(), this->path(),
                this->staticInterfaceName(), QLatin1String("TrustFingerprint"));
        callMessage << QVariant::fromValue(account) << QVariant::fromValue(contactName) << QVariant::fromValue(fingerprint) << QVariant::fromValue(trust);
        return this->connection().asyncCall(callMessage, timeout);
    }

    /**
     * Begins a call to the D-Bus method \c ForgetFingerprint on the remote object.
     *
     * Forget fingerprint romoving it from the list of known fingerprints
     *
     * Note that \a timeout is ignored as of now. It will be used once
     * http://bugreports.qt.nokia.com/browse/QTBUG-11775 is fixed.
     *
     * \param timeout The timeout in milliseconds.
     */
    inline QDBusPendingReply<> ForgetFingerprint(const QDBusObjectPath& account, const QString& contactName, const QString& fingerprint, int timeout = -1)
    {
        if (!invalidationReason().isEmpty()) {
            return QDBusPendingReply<>(QDBusMessage::createError(
                invalidationReason(),
                invalidationMessage()
            ));
        }

        QDBusMessage callMessage = QDBusMessage::createMethodCall(this->service(), this->path(),
                this->staticInterfaceName(), QLatin1String("ForgetFingerprint"));
        callMessage << QVariant::fromValue(account) << QVariant::fromValue(contactName) << QVariant::fromValue(fingerprint);
        return this->connection().asyncCall(callMessage, timeout);
    }

Q_SIGNALS:
    /**
     * Represents the signal \c ProxyConnected on the remote object.
     *
     * Signals that a proxy has been connected
     *
     * \param proxy
     *
     *     The object path of the connected proxy
     */
    void ProxyConnected(const QDBusObjectPath& proxy);

    /**
     * Represents the signal \c ProxyDisconnected on the remote object.
     *
     * Signals that a proxy has been disconnected
     *
     * \param proxy
     *
     *     The object path of the disconnectd proxy type
     */
    void ProxyDisconnected(const QDBusObjectPath& proxy);

    /**
     * Represents the signal \c KeyGenerationStarted on the remote object.
     *
     * Signals that a new private key is being generated for account
     *
     * \param account
     *
     *     The account the new key is generated for
     */
    void KeyGenerationStarted(const QDBusObjectPath& account);

    /**
     * Represents the signal \c KeyGenerationFinished on the remote object.
     *
     * Signals that a new private key has just been generated for account
     *
     * \param account
     *
     *     The account the new key has been generated for
     *
     * \param error
     *
     *     %TRUE if error occured during generation
     */
    void KeyGenerationFinished(const QDBusObjectPath& account, bool error);

protected:
    void invalidate(Tp::DBusProxy *, const QString &, const QString &) override;
};
}
}
Q_DECLARE_METATYPE(KTp::Client::ProxyServiceInterface*)
#endif
