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
    connect(this, SIGNAL(presenceChanged(Tp::Presence)), SLOT(onPresenceChanged(Tp::Presence)));
}

void KTp::Contact::onPresenceChanged(const Tp::Presence &presence)
{
    Q_UNUSED(presence)
    /* Temporary workaround for upstream bug https://bugs.freedesktop.org/show_bug.cgi?id=55883)
     * Close https://bugs.kde.org/show_bug.cgi?id=308217 when fixed upstream */
    Q_EMIT clientTypesChanged(clientTypes());
}

QString KTp::Contact::accountUniqueIdentifier() const
{
    if (m_accountUniqueIdentifier.isEmpty() && manager()->connection()) {
        const_cast<KTp::Contact*>(this)->m_accountUniqueIdentifier = manager()->connection()->property("accountUID").toString();
    }
    return m_accountUniqueIdentifier;
}

QString KTp::Contact::uri() const
{
    // so real ID will look like
    // ktp://gabble/jabber/blah/asdfjwer?foo@bar.com
    // ? is used as it is not a valid character in the dbus path that makes up the account UID
    return QStringLiteral("ktp://") + accountUniqueIdentifier() + QLatin1Char('?') + id();
}

KTp::Presence KTp::Contact::presence() const
{
    if (!manager() || !manager()->connection()) {
        return Tp::Presence::offline();
    }

    return KTp::Presence(Tp::Contact::presence());
}

bool KTp::Contact::textChatCapability() const
{
    if (!manager() || !manager()->connection()) {
        return false;
    }

    return capabilities().textChats();
}

bool KTp::Contact::audioCallCapability() const
{
    if (!manager() || !manager()->connection()) {
        return false;
    }

    Tp::ConnectionPtr connection = manager()->connection();
    bool contactCanStreamAudio = CapabilitiesHackPrivate::audioCalls(
                capabilities(), connection->cmName());
    bool selfCanStreamAudio = CapabilitiesHackPrivate::audioCalls(
                connection->selfContact()->capabilities(), connection->cmName());
    return contactCanStreamAudio && selfCanStreamAudio;
}

bool KTp::Contact::videoCallCapability() const
{
    if (!manager() || !manager()->connection()) {
        return false;
    }

    Tp::ConnectionPtr connection = manager()->connection();
    bool contactCanStreamVideo = CapabilitiesHackPrivate::videoCalls(
                capabilities(), connection->cmName());
    bool selfCanStreamVideo = CapabilitiesHackPrivate::videoCalls(
                connection->selfContact()->capabilities(), connection->cmName());
    return contactCanStreamVideo && selfCanStreamVideo;
}

bool KTp::Contact::fileTransferCapability()  const
{
    if (!manager() || !manager()->connection()) {
        return false;
    }

    bool contactCanHandleFiles = capabilities().fileTransfers();
    bool selfCanHandleFiles = manager()->connection()->selfContact()->capabilities().fileTransfers();
    return contactCanHandleFiles && selfCanHandleFiles;
}

bool KTp::Contact::collaborativeEditingCapability() const
{
    if (!manager() || !manager()->connection()) {
        return false;
    }

    static const QString collab(QLatin1String("infinote"));
    bool selfCanShare = manager()->connection()->selfContact()->capabilities().streamTubes(collab);
    bool otherCanShare = capabilities().streamTubes(collab);
    return selfCanShare && otherCanShare;
}

QStringList KTp::Contact::dbusTubeServicesCapability() const
{
    if (!manager() || !manager()->connection()) {
        return QStringList();
    }

    return getCommonElements(capabilities().dbusTubeServices(),
                             manager()->connection()->selfContact()->capabilities().dbusTubeServices());
}

QStringList KTp::Contact::streamTubeServicesCapability() const
{
    if (!manager() || !manager()->connection()) {
        return QStringList();
    }

    return getCommonElements(capabilities().streamTubeServices(),
                             manager()->connection()->selfContact()->capabilities().streamTubeServices());
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

        //if neither above succeeded, return empty QPixmap,
        //PersonsModel will return the default icon instead
        if (avatar.isNull()) {
            return QPixmap();
        }

        //insert the contact into pixmap cache for faster lookup
        QPixmapCache::insert(keyCache(), avatar);
    }

    return avatar;
}

void KTp::Contact::avatarToGray(QPixmap &avatar)
{
    QImage image = avatar.toImage();
    QImage alpha= image.alphaChannel();
    for (int i = 0; i < image.width(); ++i) {
        for (int j = 0; j < image.height(); ++j) {
            int colour = qGray(image.pixel(i, j));
            image.setPixel(i, j, qRgb(colour, colour, colour));
        }
    }
    image.setAlphaChannel(alpha);
    avatar = QPixmap::fromImage(image);
}

QString KTp::Contact::keyCache() const
{
    return id() + (presence().type() == Tp::ConnectionPresenceTypeOffline ? QLatin1String("-offline") : QLatin1String("-online"));
}

QString KTp::Contact::buildAvatarPath(const QString &avatarToken)
{
    QString cacheDir = QString::fromLatin1(qgetenv("XDG_CACHE_HOME"));
    if (cacheDir.isEmpty()) {
        cacheDir = QStringLiteral("%1/.cache").arg(QLatin1String(qgetenv("HOME")));
    }

    if (manager().isNull()) {
        return QString();
    }

    if (manager()->connection().isNull()) {
        return QString();
    }

    Tp::ConnectionPtr conn = manager()->connection();
    QString path = QStringLiteral("%1/telepathy/avatars/%2/%3").
        arg(cacheDir).arg(conn->cmName()).arg(conn->protocolName());

    QString avatarFileName = QStringLiteral("%1/%2").arg(path).arg(Tp::escapeAsIdentifier(avatarToken));

    return avatarFileName;
}

void KTp::Contact::invalidateAvatarCache()
{
    QPixmapCache::remove(id() + QLatin1String("-offline"));
    QPixmapCache::remove(id() + QLatin1String("-online"));
}

QStringList KTp::Contact::getCommonElements(const QStringList &list1, const QStringList &list2)
{
    /* QStringList::contains(QString) perform iterative comparsion, so there is no reason
     * to select smaller list as base for this cycle. */
    QStringList commonElements;
    Q_FOREACH(const QString &i, list1) {
        if (list2.contains(i)) {
            commonElements << i;
        }
    }
    return commonElements;
}
