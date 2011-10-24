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

ErrorDictionary* ErrorDictionary::m_instance = NULL;

ErrorDictionary* ErrorDictionary::instance()
{
    if (!m_instance) {
        m_instance = new ErrorDictionary(0);
    }

    return m_instance;
}


ErrorDictionary::ErrorDictionary(QObject* parent = 0)
    : QObject(parent)
{
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.AlreadyConnected"),
             i18n("Looks like you are already connected from other location and the server does not allow multiple connections"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.AuthenticationFailed"),
             i18n("Authentication of your account failed (is your password correct?)"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Busy"),
             i18n("The channel is too busy now to process your request. Try again in a few minutes"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cancelled"),
             i18n("The connection was canceled on your request"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Expired"),
             i18n("The SSL/TLS certificate received from server is expired"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.FingerprintMismatch"),
             i18n("The SSL/TLS certificate received from server has different fingerprint than expected"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.HostnameMismatch"),
             i18n("The SSL/TLS certificate received from server has different hostname than expected"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Insecure"),
             i18n("The SSL/TLS certificate received from server uses too weak encryption"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Invalid"),
             i18n("The SSL/TLS certificate received from server is invalid"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.LimitExceeded"),
             i18n("The SSL/TLS certificate received from server has exceeded length limit"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.NotActivated"),
             i18n("The SSL/TLS certificate received from server has not been activated yet"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.NotProvided"),
             i18n("The server did not provide any SSL/TLS certificate"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Revoked"),
             i18n("The SSL/TLS certificate received from server has been revoked"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.SelfSigned"),
             i18n("The SSL/TLS certificate received from server was self-signed by the server and is untrusted"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Cert.Untrusted"),
             i18n("The SSL/TLS certificate received from server was not signed by a trusted certificate authority"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.Banned"),
             i18n("You have been banned from the channel"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.Full"),
             i18n("The channel is full"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.InviteOnly"),
             i18n("The channel is invite-only"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Channel.Kicked"),
             i18n("You have been kicked from the channel"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Confused"),
             i18n("Congratulate yourself - you just reached an impossible situation"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionFailed"),
             i18n("Could not estabilish connection"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionLost"),
             i18n("Connection to server was lost"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionRefused"),
             i18n("Server refused your connection"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ConnectionReplaced"),
             i18n("Somewhere there is another client connecting with your account and your current connection is lost"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Disconnected"),
             i18n("You have been disconnected"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.DoesNotExist"),
             i18n("You apparently do not exist "));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.EmergencyCallsNotSupported"),
             i18n("Sorry, emergency calls are not supported"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.EncryptionError"),
             i18n("An encryption error has occured"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.EncryptionNotAvailable"),
             i18n("Requested encryption is not available"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.InsufficientBalance"),
             i18n("Your call credit is too low"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.InvalidArgument"),
             i18n("An invalid argument was provided"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.InvalidHandle"),
             i18n("The specified handle is unknown on this channel"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Media.CodecsIncompatible"),
             i18n("You appear to not have any common codec with the other side"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Media.StreamingError"),
             i18n("A streaming error has occured (probably not your network related)"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Media.UnsupportedType"),
             i18n("The requested media stream type is not available"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NetworkError"),
             i18n("There appears to be a problem with your network, check your connection"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NoAnswer"),
             i18n("You were removed from the channel because you did not respond")); //FIXME: this sound bad
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotAvailable"),
             i18n("This capability is not available"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotCapable"),
             i18n("The contact does not have the requested capabilities"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotImplemented"),
             i18n("This operation is not implemented"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotYet"),
             i18n("This operation is not yet implemented"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.NotYours"),
             i18n("The requested channel is already being handled by some other process"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Offline"),
             i18n("This operation is unavailable as the contact is offline"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.PermissionDenied"),
             i18n("You are not permitted to perform this operation"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.PickedUpElsewhere"),
             i18n("Current call was picked up by another resource"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.RegistrationExists"),
             i18n("Account with this username already exists"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Rejected"),
             i18n("The receiver rejected your call"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ResourceUnavailable"),
             i18n("There are insufficient resources (like free memory) to finish the operation at the moment"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ServiceBusy"),
             i18n("Your request hit a busy resource somewhere along the way and the operation was unable to finish"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.ServiceConfused"),
             i18n("An internal error has occured (known as the 'Confused service error')"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.SoftwareUpgradeRequired"),
             i18n("You are using too old software. Please try updating all Telepathy packages to a newer version"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.Terminated"),
             i18n("The channel was terminated for no apparent reason"));
    m_dict.insert(QLatin1String("org.freedesktop.Telepathy.Error.WouldBreakAnonymity"),
             i18n("This operation can not be finished as it would break your anonymity request"));
}

ErrorDictionary::~ErrorDictionary()
{

}

QString ErrorDictionary::displayErrorMessage(const QString& dbusErrorName) const
{
    return m_dict.value(dbusErrorName);
}
