/*
    Telepathy error dictionary - usable strings for dbus messages
    Copyright (C) 2011  Martin Klapetek <martin.klapetek@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "error-dictionary.h"

#include <KLocalizedString>
#include <KGlobal>
#include <KLocale>
#include <KDebug>

ErrorDictionary* ErrorDictionary::s_instance = NULL;

ErrorDictionary* ErrorDictionary::instance()
{
    if (!s_instance) {
        s_instance = new ErrorDictionary(0);
    }

    return s_instance;
}


ErrorDictionary::ErrorDictionary(QObject* parent = 0)
    : QObject(parent)
{
    KGlobal::locale()->insertCatalog(QLatin1String("telepathy-common-internals"));

    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.AlreadyConnected"),
             i18nc("Verbose user visible error string", "Looks like you are already connected from other location and the server does not allow multiple connections"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.AuthenticationFailed"),
             i18nc("Verbose user visible error string", "Authentication of your account failed (is your password correct?)"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Busy"),
             i18nc("Verbose user visible error string", "The channel is too busy now to process your request. Try again in a few minutes"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cancelled"),
             i18nc("Verbose user visible error string", "The connection was canceled on your request"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Expired"),
             i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server is expired"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.FingerprintMismatch"),
             i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server has different fingerprint than expected"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.HostnameMismatch"),
             i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server has different hostname than expected"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Insecure"),
             i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server uses too weak encryption"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Invalid"),
             i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server is invalid"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.LimitExceeded"),
             i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server has exceeded length limit"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.NotActivated"),
             i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server has not been activated yet"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.NotProvided"),
             i18nc("Verbose user visible error string", "The server did not provide any SSL/TLS certificate"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Revoked"),
             i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server has been revoked"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.SelfSigned"),
             i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server was self-signed by the server and is untrusted"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Untrusted"),
             i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server was not signed by a trusted certificate authority"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.Banned"),
             i18nc("Verbose user visible error string", "You have been banned from the channel"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.Full"),
             i18nc("Verbose user visible error string", "The channel is full"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.InviteOnly"),
             i18nc("Verbose user visible error string", "The channel is invite-only"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.Kicked"),
             i18nc("Verbose user visible error string", "You have been kicked from the channel"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Confused"),
             i18nc("Verbose user visible error string", "Congratulate yourself - you just reached an impossible situation"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionFailed"),
             i18nc("Verbose user visible error string", "Could not establish connection"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionLost"),
             i18nc("Verbose user visible error string", "Connection to server was lost"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionRefused"),
             i18nc("Verbose user visible error string", "Server refused your connection"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionReplaced"),
             i18nc("Verbose user visible error string", "Somewhere there is another client connecting with your account and your current connection is lost"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Disconnected"),
             i18nc("Verbose user visible error string", "You have been disconnected"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.DoesNotExist"),
             i18nc("Verbose user visible error string", "You apparently do not exist "));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.EmergencyCallsNotSupported"),
             i18nc("Verbose user visible error string", "Sorry, emergency calls are not supported"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.EncryptionError"),
             i18nc("Verbose user visible error string", "An encryption error has occurred"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.EncryptionNotAvailable"),
             i18nc("Verbose user visible error string", "Requested encryption is not available"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.InsufficientBalance"),
             i18nc("Verbose user visible error string", "Your call credit is too low"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.InvalidArgument"),
             i18nc("Verbose user visible error string", "An invalid argument was provided"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.InvalidHandle"),
             i18nc("Verbose user visible error string", "The specified handle is unknown on this channel"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Media.CodecsIncompatible"),
             i18nc("Verbose user visible error string", "You appear to not have any common codec with the other side"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Media.StreamingError"),
             i18nc("Verbose user visible error string", "A streaming error has occurred (probably not your network related)"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Media.UnsupportedType"),
             i18nc("Verbose user visible error string", "The requested media stream type is not available"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NetworkError"),
             i18nc("Verbose user visible error string", "There appears to be a problem with your network, check your connection"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NoAnswer"),
             i18nc("Verbose user visible error string", "You were removed from the channel because you did not respond")); //FIXME: this sound bad
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotAvailable"),
             i18nc("Verbose user visible error string", "This capability is not available"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotCapable"),
             i18nc("Verbose user visible error string", "The contact does not have the requested capabilities"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotImplemented"),
             i18nc("Verbose user visible error string", "This operation is not implemented"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotYet"),
             i18nc("Verbose user visible error string", "This operation is not yet implemented"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotYours"),
             i18nc("Verbose user visible error string", "The requested channel is already being handled by some other process"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Offline"),
             i18nc("Verbose user visible error string", "This operation is unavailable as the contact is offline"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.PermissionDenied"),
             i18nc("Verbose user visible error string", "You are not permitted to perform this operation"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.PickedUpElsewhere"),
             i18nc("Verbose user visible error string", "Current call was picked up by another resource"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.RegistrationExists"),
             i18nc("Verbose user visible error string", "Account with this username already exists"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Rejected"),
             i18nc("Verbose user visible error string", "The receiver rejected your call"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ResourceUnavailable"),
             i18nc("Verbose user visible error string", "There are insufficient resources (like free memory) to finish the operation at the moment"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ServiceBusy"),
             i18nc("Verbose user visible error string", "Your request hit a busy resource somewhere along the way and the operation was unable to finish"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ServiceConfused"),
             i18nc("Verbose user visible error string", "An internal error has occurred (known as the 'Confused service error')"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.SoftwareUpgradeRequired"),
             i18nc("Verbose user visible error string", "You are using too old software. Please try updating all Telepathy packages to a newer version"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Terminated"),
             i18nc("Verbose user visible error string", "The channel was terminated for no apparent reason"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.WouldBreakAnonymity"),
             i18nc("Verbose user visible error string", "This operation can not be finished as it would break your anonymity request"));
    m_verboseDict.insert(QLatin1String("org.freedesktop.DBus.Error.NoReply"),
             i18nc("Verbose user visible error string", "Some of the IM components are not working correctly (and your system does not tell us which one)"));

    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.AlreadyConnected"),
             i18nc("Short user visible error string", "Connected elsewhere"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.AuthenticationFailed"),
             i18nc("Short user visible error string", "Authentication failed"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Busy"),
             i18nc("Short user visible error string", "Channel too busy"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cancelled"),
             i18nc("Short user visible error string", "Cancelled by user"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Expired"),
             i18nc("Short user visible error string", "Certificate is expired"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.FingerprintMismatch"),
             i18nc("Short user visible error string", "Wrong certificate fingerprint"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.HostnameMismatch"),
             i18nc("Short user visible error string", "Wrong certificate hostname"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Insecure"),
             i18nc("Short user visible error string", "Too weak certificate encryption"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Invalid"),
             i18nc("Short user visible error string", "Invalid certificate"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.LimitExceeded"),
             i18nc("Short user visible error string", "Certificate length limit exceeded"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.NotActivated"),
             i18nc("Short user visible error string", "Certificate not yet active"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.NotProvided"),
             i18nc("Short user visible error string", "No certificate from server"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Revoked"),
             i18nc("Short user visible error string", "Certificate revoked"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.SelfSigned"),
             i18nc("Short user visible error string", "Untrusted self-signed certificate"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Untrusted"),
             i18nc("Short user visible error string", "Untrusted certificate"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.Banned"),
             i18nc("Short user visible error string", "Banned from channel"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.Full"),
             i18nc("Short user visible error string", "The channel is full"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.InviteOnly"),
             i18nc("Short user visible error string", "The channel is invite-only"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.Kicked"),
             i18nc("Short user visible error string", "Kicked from channel"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Confused"),
             i18nc("Short user visible error string", "Something's seriously wrong"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionFailed"),
             i18nc("Short user visible error string", "Connection failed"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionLost"),
             i18nc("Short user visible error string", "Connection lost"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionRefused"),
             i18nc("Short user visible error string", "Connection refused"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionReplaced"),
             i18nc("Short user visible error string", "Connection stolen by other account"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Disconnected"),
             i18nc("Short user visible error string", "Disconnected"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.DoesNotExist"),
             i18nc("Short user visible error string", "You apparently do not exist"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.EmergencyCallsNotSupported"),
             i18nc("Short user visible error string", "Emergency calls unsupported"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.EncryptionError"),
             i18nc("Short user visible error string", "Encryption error"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.EncryptionNotAvailable"),
             i18nc("Short user visible error string", "Encryption not available"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.InsufficientBalance"),
             i18nc("Short user visible error string", "Low call credit"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.InvalidArgument"),
             i18nc("Short user visible error string", "Invalid argument"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.InvalidHandle"),
             i18nc("Short user visible error string", "Unknown handle"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Media.CodecsIncompatible"),
             i18nc("Short user visible error string", "No compatible codec"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Media.StreamingError"),
             i18nc("Short user visible error string", "Streaming error"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Media.UnsupportedType"),
             i18nc("Short user visible error string", "Unsupported stream type"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NetworkError"),
             i18nc("Short user visible error string", "Network error"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NoAnswer"),
             i18nc("Short user visible error string", "Removed from channel"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotAvailable"),
             i18nc("Short user visible error string", "Capability not available"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotCapable"),
             i18nc("Short user visible error string", "Contact incapable"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotImplemented"),
             i18nc("Short user visible error string", "Operation not implemented"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotYet"),
             i18nc("Short user visible error string", "Operation not yet implemented"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotYours"),
             i18nc("Short user visible error string", "Channel already being handled"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Offline"),
             i18nc("Short user visible error string", "Contact is offline"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.PermissionDenied"),
             i18nc("Short user visible error string", "Operation not permitted"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.PickedUpElsewhere"),
             i18nc("Short user visible error string", "Call picked by other resource"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.RegistrationExists"),
             i18nc("Short user visible error string", "Username already taken"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Rejected"),
             i18nc("Short user visible error string", "Call rejected"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ResourceUnavailable"),
             i18nc("Short user visible error string", "Insufficient resources"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ServiceBusy"),
             i18nc("Short user visible error string", "Service is busy"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ServiceConfused"),
             i18nc("Short user visible error string", "Internal error"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.SoftwareUpgradeRequired"),
             i18nc("Short user visible error string", "Software upgrade required"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Terminated"),
             i18nc("Short user visible error string", "Channel terminated"));
    m_shortDict.insert(QLatin1String("org.freedesktop.Telepathy.Error.WouldBreakAnonymity"),
             i18nc("Short user visible error string", "Anonymity break possible"));
    m_shortDict.insert(QLatin1String("org.freedesktop.DBus.Error.NoReply"),
             i18nc("Short user visible error string", "Internal component error"));
}

ErrorDictionary::~ErrorDictionary()
{

}

QString ErrorDictionary::displayVerboseErrorMessage(const QString& dbusErrorName) const
{
    if (!m_verboseDict.contains(dbusErrorName)) {
        return i18nc("User visible error string", "An unknown error was encountered (%1), please report this", dbusErrorName);
    } else {
        return m_verboseDict.value(dbusErrorName);
    }
}

QString ErrorDictionary::displayShortErrorMessage(const QString& dbusErrorName) const
{
    if (!m_shortDict.contains(dbusErrorName)) {
        //print the error so users can send it in
        kWarning() << "Unknown error encountered:" << dbusErrorName;
        return i18nc("User visible error string", "Unknown error");
    } else {
        return m_shortDict.value(dbusErrorName);
    }
}
