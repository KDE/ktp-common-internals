/*
* Copyright (C) 2012 David Edmundson <kde@davidedmundson.co.uk>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "contact.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ContactCapabilities>
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/Utils>

#include <QBitmap>
#include <QPixmap>
#include <QPixmapCache>

#include <KIconLoader>
#include <KConfigGroup>
#include <KConfig>

#include "capabilities-hack-private.h"

KTp::Contact::Contact(Tp::ContactManager *manager, const Tp::ReferencedHandles &handle, const Tp::Features &requestedFeatures, const QVariantMap &attributes)
    : Tp::Contact(manager, handle, requestedFeatures, attributes)
{
    connect(manager->connection().data(), SIGNAL(destroyed()), SIGNAL(invalidated()));
    connect(manager->connection().data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)), SIGNAL(invalidated()));
    connect(this, SIGNAL(avatarTokenChanged(QString)), SLOT(invalidateAvatarCache()));
    connect(this, SIGNAL(avatarDataChanged(Tp::AvatarData)), SLOT(invalidateAvatarCache()));
}

KTp::Presence KTp::Contact::presence() const
{
    return KTp::Presence(Tp::Contact::presence());
}

bool KTp::Contact::audioCallCapability() const
{
    if (! manager()->connection()) {
        return false;
    }
    Tp::ConnectionPtr connection = manager()->connection();
    if (connection) {
        bool contactCanStreamAudio = CapabilitiesHackPrivate::audioCalls(
                                         capabilities(), connection->cmName());
        bool selfCanStreamAudio = CapabilitiesHackPrivate::audioCalls(
                                      connection->selfContact()->capabilities(), connection->cmName());
        return contactCanStreamAudio && selfCanStreamAudio;
    }
    return false;
}

bool KTp::Contact::videoCallCapability() const
{
    if (! manager()->connection()) {
        return false;
    }
    Tp::ConnectionPtr connection = manager()->connection();
    if (connection) {
        bool contactCanStreamVideo = CapabilitiesHackPrivate::videoCalls(
                                         capabilities(), connection->cmName());
        bool selfCanStreamVideo = CapabilitiesHackPrivate::videoCalls(
                                      connection->selfContact()->capabilities(), connection->cmName());
        return contactCanStreamVideo && selfCanStreamVideo;
    }

    return false;
}

bool KTp::Contact::fileTransferCapability()  const
{
    if (! manager()->connection()) {
        return false;
    }
    if (manager()->connection()) {
        bool contactCanHandleFiles = capabilities().fileTransfers();
        bool selfCanHandleFiles = manager()->connection()->selfContact()->capabilities().fileTransfers();
        return contactCanHandleFiles && selfCanHandleFiles;
    }

    return false;
}

QStringList KTp::Contact::clientTypes() const
{
    /* Temporary workaround for upstream bug https://bugs.freedesktop.org/show_bug.cgi?id=55883)
     * Close https://bugs.kde.org/show_bug.cgi?id=308217 when fixed upstream */
    if (Tp::Contact::presence().type() == Tp::ConnectionPresenceTypeOffline) {
        return QStringList();
    }

    //supress any errors trying to access ClientTypes when we don't have them
    if (! actualFeatures().contains(Tp::Contact::FeatureClientTypes)) {
        return QStringList();
    }

    return Tp::Contact::clientTypes();
}

QPixmap KTp::Contact::avatarPixmap()
{
    QPixmap avatar;

    //check pixmap cache for the avatar, if not present, load the avatar
    if (!QPixmapCache::find(keyCache(), avatar)){
        QString file = avatarData().fileName;

        //if contact does not provide path, let's see if we have avatar for the stored token
        if (file.isEmpty()) {
            KConfig config(QLatin1String("ktelepathy-avatarsrc"));
            KConfigGroup avatarTokenGroup = config.group(id());
            QString avatarToken = avatarTokenGroup.readEntry(QLatin1String("avatarToken"));
            //only bother loading the pixmap if the token is not empty
            if (!avatarToken.isEmpty()) {
                avatar.load(buildAvatarPath(avatarToken));
            }
        } else {
            avatar.load(file);
        }

        //if neither above succeeded, we need to load the icon
        if (avatar.isNull()) {
            avatar = KIconLoader::global()->loadIcon(QLatin1String("im-user"), KIconLoader::NoGroup, 96);
        }

        //if the contact is offline, gray it out
        if (presence().type() == Tp::ConnectionPresenceTypeOffline) {
            avatarToGray(avatar);
        }

        //insert the contact into pixmap cache for faster lookup
        QPixmapCache::insert(keyCache(), avatar);
    }

    return avatar;
}

void KTp::Contact::avatarToGray(QPixmap &avatar)
{
    QImage image = avatar.toImage();
    QPixmap alpha= avatar.alphaChannel();
    for (int i = 0; i < image.width(); ++i) {
        for (int j = 0; j < image.height(); ++j) {
            int colour = qGray(image.pixel(i, j));
            image.setPixel(i, j, qRgb(colour, colour, colour));
        }
    }
    avatar = avatar.fromImage(image);
    avatar.setAlphaChannel(alpha);
}

QString KTp::Contact::keyCache() const
{
    return id() + (presence().type() == Tp::ConnectionPresenceTypeOffline ? QLatin1String("-offline") : QLatin1String("-online"));
}

QString KTp::Contact::buildAvatarPath(const QString &avatarToken)
{
    QString cacheDir = QString::fromLatin1(qgetenv("XDG_CACHE_HOME"));
    if (cacheDir.isEmpty()) {
        cacheDir = QString::fromLatin1("%1/.cache").arg(QLatin1String(qgetenv("HOME")));
    }

    if (manager().isNull()) {
        return QString();
    }

    if (manager()->connection().isNull()) {
        return QString();
    }

    Tp::ConnectionPtr conn = manager()->connection();
    QString path = QString::fromLatin1("%1/telepathy/avatars/%2/%3").
        arg(cacheDir).arg(conn->cmName()).arg(conn->protocolName());

    QString avatarFileName = QString::fromLatin1("%1/%2").arg(path).arg(Tp::escapeAsIdentifier(avatarToken));

    return avatarFileName;
}

void KTp::Contact::invalidateAvatarCache()
{
    QPixmapCache::remove(id() + QLatin1String("-offline"));
    QPixmapCache::remove(id() + QLatin1String("-online"));
}
