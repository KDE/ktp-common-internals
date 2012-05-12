/*
    Telepathy error dictionary - usable strings for dbus messages
    Copyright (C) 2011  Martin Klapetek <martin.klapetek@gmail.com>
    Copyright (C) 2011  Dario Freddi <dario.freddi@collabora.com>

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

#include <TelepathyQt/Constants>

#include <KLocalizedString>
#include <KGlobal>
#include <KLocale>
#include <KDebug>

namespace KTp
{

QString ErrorDictionary::displayVerboseErrorMessage(const QString& dbusErrorName)
{
    if (dbusErrorName == QLatin1String(TP_QT_ERROR_ALREADY_CONNECTED)) {
        return i18nc("Verbose user visible error string", "Looks like you are already connected from other location and the server does not allow multiple connections");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_AUTHENTICATION_FAILED)) {
        return i18nc("Verbose user visible error string", "Authentication of your account failed (is your password correct?)");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_BUSY)) {
        return i18nc("Verbose user visible error string", "The channel is too busy now to process your request. Try again in a few minutes");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CANCELLED)) {
        return i18nc("Verbose user visible error string", "The connection was canceled on your request");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_EXPIRED)) {
        return i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server is expired");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_FINGERPRINT_MISMATCH)) {
        return i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server has different fingerprint than expected");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_HOSTNAME_MISMATCH)) {
        return i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server has different hostname than expected");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_INSECURE)) {
        return i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server uses too weak encryption");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_INVALID)) {
        return i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server is invalid");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_LIMIT_EXCEEDED)) {
        return i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server has exceeded length limit");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_NOT_ACTIVATED)) {
        return i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server has not been activated yet");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_NOT_PROVIDED)) {
        return i18nc("Verbose user visible error string", "The server did not provide any SSL/TLS certificate");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_REVOKED)) {
        return i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server has been revoked");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_SELF_SIGNED)) {
        return i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server was self-signed by the server and is untrusted");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_UNTRUSTED)) {
        return i18nc("Verbose user visible error string", "The SSL/TLS certificate received from server was not signed by a trusted certificate authority");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CHANNEL_BANNED)) {
        return i18nc("Verbose user visible error string", "You have been banned from the channel");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CHANNEL_FULL)) {
        return i18nc("Verbose user visible error string", "The channel is full");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CHANNEL_INVITE_ONLY)) {
        return i18nc("Verbose user visible error string", "The channel is invite-only");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CHANNEL_KICKED)) {
        return i18nc("Verbose user visible error string", "You have been kicked from the channel");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CONFUSED)) {
        return i18nc("Verbose user visible error string", "Congratulate yourself - you just reached an impossible situation");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CONNECTION_FAILED)) {
        return i18nc("Verbose user visible error string", "Could not establish connection");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CONNECTION_LOST)) {
        return i18nc("Verbose user visible error string", "Connection to server was lost");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CONNECTION_REFUSED)) {
        return i18nc("Verbose user visible error string", "Server refused your connection");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CONNECTION_REPLACED)) {
        return i18nc("Verbose user visible error string", "Somewhere there is another client connecting with your account and your current connection is lost");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_DISCONNECTED)) {
        return i18nc("Verbose user visible error string", "You have been disconnected");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_DOES_NOT_EXIST)) {
        return i18nc("Verbose user visible error string", "You apparently do not exist");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_EMERGENCY_CALLS_NOT_SUPPORTED)) {
        return i18nc("Verbose user visible error string", "Sorry, emergency calls are not supported");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_ENCRYPTION_ERROR)) {
        return i18nc("Verbose user visible error string", "An encryption error has occurred");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_ENCRYPTION_NOT_AVAILABLE)) {
        return i18nc("Verbose user visible error string", "Requested encryption is not available");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_INVALID_ARGUMENT)) {
        return i18nc("Verbose user visible error string", "An invalid argument was provided");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_INVALID_HANDLE)) {
        return i18nc("Verbose user visible error string", "The specified handle is unknown on this channel");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NETWORK_ERROR)) {
        return i18nc("Verbose user visible error string", "There appears to be a problem with your network, check your connection");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NO_ANSWER)) {
        return i18nc("Verbose user visible error string", "You were removed from the channel because you did not respond"); //FIXME: this sound bad
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NOT_AVAILABLE)) {
        return i18nc("Verbose user visible error string", "This capability is not available");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NOT_CAPABLE)) {
        return i18nc("Verbose user visible error string", "The contact does not have the requested capabilities");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NOT_IMPLEMENTED)) {
        return i18nc("Verbose user visible error string", "This operation is not implemented");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NOT_YET)) {
        return i18nc("Verbose user visible error string", "This operation is not yet implemented");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NOT_YOURS)) {
        return i18nc("Verbose user visible error string", "The requested channel is already being handled by some other process");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_OFFLINE)) {
        return i18nc("Verbose user visible error string", "This operation is unavailable as the contact is offline");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_PERMISSION_DENIED)) {
        return i18nc("Verbose user visible error string", "You are not permitted to perform this operation");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_PICKED_UP_ELSEWHERE)) {
        return i18nc("Verbose user visible error string", "Current call was picked up by another resource");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_REGISTRATION_EXISTS)) {
        return i18nc("Verbose user visible error string", "Account with this username already exists");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_REJECTED)) {
        return i18nc("Verbose user visible error string", "The receiver rejected your call");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_RESOURCE_UNAVAILABLE)) {
        return i18nc("Verbose user visible error string", "There are insufficient resources (like free memory) to finish the operation at the moment");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_SERVICE_BUSY)) {
        return i18nc("Verbose user visible error string", "Your request hit a busy resource somewhere along the way and the operation was unable to finish");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_SERVICE_CONFUSED)) {
        return i18nc("Verbose user visible error string", "An internal error has occurred (known as the 'Confused service error')");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_SOFTWARE_UPGRADE_REQUIRED)) {
        return i18nc("Verbose user visible error string", "You are using too old software. Please try updating all Telepathy packages to a newer version");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_TERMINATED)) {
        return i18nc("Verbose user visible error string", "The channel was terminated for no apparent reason");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_WOULD_BREAK_ANONYMITY)) {
        return i18nc("Verbose user visible error string", "This operation can not be finished as it would break your anonymity request");
    } else if (dbusErrorName == QLatin1String("org.freedesktop.DBus.Error.NoReply")) {
        return i18nc("Verbose user visible error string", "Some of the IM components are not working correctly");
    } else {
        return i18nc("User visible error string", "An unknown error was encountered (%1), please report this", dbusErrorName);
    }
}

QString ErrorDictionary::displayShortErrorMessage(const QString& dbusErrorName)
{
    if (dbusErrorName == QLatin1String(TP_QT_ERROR_ALREADY_CONNECTED)) {
        return i18nc("Short user visible error string", "Connected elsewhere");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_AUTHENTICATION_FAILED)) {
        return i18nc("Short user visible error string", "Authentication failed");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_BUSY)) {
        return i18nc("Short user visible error string", "Channel too busy");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CANCELLED)) {
        return i18nc("Short user visible error string", "Cancelled by user");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_EXPIRED)) {
        return i18nc("Short user visible error string", "Certificate is expired");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_FINGERPRINT_MISMATCH)) {
        return i18nc("Short user visible error string", "Wrong certificate fingerprint");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_HOSTNAME_MISMATCH)) {
        return i18nc("Short user visible error string", "Wrong certificate hostname");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_INSECURE)) {
        return i18nc("Short user visible error string", "Too weak certificate encryption");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_INVALID)) {
        return i18nc("Short user visible error string", "Invalid certificate");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_LIMIT_EXCEEDED)) {
        return i18nc("Short user visible error string", "Certificate length limit exceeded");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_NOT_ACTIVATED)) {
        return i18nc("Short user visible error string", "Certificate not yet active");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_NOT_PROVIDED)) {
        return i18nc("Short user visible error string", "No certificate from server");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_REVOKED)) {
        return i18nc("Short user visible error string", "Certificate revoked");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_SELF_SIGNED)) {
        return i18nc("Short user visible error string", "Untrusted self-signed certificate");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CERT_UNTRUSTED)) {
        return i18nc("Short user visible error string", "Untrusted certificate");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CHANNEL_BANNED)) {
        return i18nc("Short user visible error string", "Banned from channel");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CHANNEL_FULL)) {
        return i18nc("Short user visible error string", "The channel is full");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CHANNEL_INVITE_ONLY)) {
        return i18nc("Short user visible error string", "The channel is invite-only");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CHANNEL_KICKED)) {
        return i18nc("Short user visible error string", "Kicked from channel");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CONFUSED)) {
        return i18nc("Short user visible error string", "Something's seriously wrong");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CONNECTION_FAILED)) {
        return i18nc("Short user visible error string", "Connection failed");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CONNECTION_LOST)) {
        return i18nc("Short user visible error string", "Connection lost");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CONNECTION_REFUSED)) {
        return i18nc("Short user visible error string", "Connection refused");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_CONNECTION_REPLACED)) {
        return i18nc("Short user visible error string", "Connection stolen by other account");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_DISCONNECTED)) {
        return i18nc("Short user visible error string", "Disconnected");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_DOES_NOT_EXIST)) {
        return i18nc("Short user visible error string", "You apparently do not exist");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_EMERGENCY_CALLS_NOT_SUPPORTED)) {
        return i18nc("Short user visible error string", "Emergency calls unsupported");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_ENCRYPTION_ERROR)) {
        return i18nc("Short user visible error string", "Encryption error");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_ENCRYPTION_NOT_AVAILABLE)) {
        return i18nc("Short user visible error string", "Encryption not available");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_INVALID_ARGUMENT)) {
        return i18nc("Short user visible error string", "Invalid argument");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_INVALID_HANDLE)) {
        return i18nc("Short user visible error string", "Unknown handle");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NETWORK_ERROR)) {
        return i18nc("Short user visible error string", "Network error");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NO_ANSWER)) {
        return i18nc("Short user visible error string", "Removed from channel");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NOT_AVAILABLE)) {
        return i18nc("Short user visible error string", "Capability not available");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NOT_CAPABLE)) {
        return i18nc("Short user visible error string", "Contact incapable");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NOT_IMPLEMENTED)) {
        return i18nc("Short user visible error string", "Operation not implemented");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NOT_YET)) {
        return i18nc("Short user visible error string", "Operation not yet implemented");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_NOT_YOURS)) {
        return i18nc("Short user visible error string", "Channel already being handled");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_OFFLINE)) {
        return i18nc("Short user visible error string", "Contact is offline");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_PERMISSION_DENIED)) {
        return i18nc("Short user visible error string", "Operation not permitted");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_PICKED_UP_ELSEWHERE)) {
        return i18nc("Short user visible error string", "Call picked by other resource");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_REGISTRATION_EXISTS)) {
        return i18nc("Short user visible error string", "Username already taken");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_REJECTED)) {
        return i18nc("Short user visible error string", "Call rejected");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_RESOURCE_UNAVAILABLE)) {
        return i18nc("Short user visible error string", "Insufficient resources");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_SERVICE_BUSY)) {
        return i18nc("Short user visible error string", "Service is busy");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_SERVICE_CONFUSED)) {
        return i18nc("Short user visible error string", "Internal error");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_SOFTWARE_UPGRADE_REQUIRED)) {
        return i18nc("Short user visible error string", "Software upgrade required");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_TERMINATED)) {
        return i18nc("Short user visible error string", "Channel terminated");
    } else if (dbusErrorName == QLatin1String(TP_QT_ERROR_WOULD_BREAK_ANONYMITY)) {
        return i18nc("Short user visible error string", "Anonymity break possible");
    } else if (dbusErrorName == QLatin1String("org.freedesktop.DBus.Error.NoReply")) {
        return i18nc("Short user visible error string", "Internal component error");
    } else {
        //print the error so users can send it in
        kWarning() << "Unknown error encountered:" << dbusErrorName;
        return i18nc("User visible error string", "Unknown error");
    }
}

}
