/*
    Copyright (C) 2013  Martin Klapetek <mklapetek@kde.org>

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


#include "kpeopletranslationproxy.h"

#include <KPeople/PersonsModel>

#include <kpeople/personpluginmanager.h>

#include "KTp/im-persons-data-source.h"
#include "KTp/types.h"

#include <KDebug>
#include <KIconLoader>

#include <QPixmapCache>

using namespace KPeople;


KPeopleTranslationProxy::KPeopleTranslationProxy(QObject *parent)
    : QIdentityProxyModel(parent)
{
}

KPeopleTranslationProxy::~KPeopleTranslationProxy()
{
}

QVariant KPeopleTranslationProxy::data(const QModelIndex &proxyIndex, int role) const
{
    if (!proxyIndex.isValid()) {
        return QVariant();
    }

    IMPersonsDataSource *imPlugin = qobject_cast<IMPersonsDataSource*>(PersonPluginManager::presencePlugin());

    if (!imPlugin) {
        kWarning() << "No imPlugin";
        return QVariant();
    }

    switch (role) {
        case KTp::ContactPresenceTypeRole:
            return translatePresence(mapToSource(proxyIndex).data(PersonsModel::PresenceTypeRole));
        case KTp::ContactPresenceIconRole:
            return mapToSource(proxyIndex).data(PersonsModel::PresenceIconNameRole);
        case KTp::ContactPresenceNameRole:
            return mapToSource(proxyIndex).data(PersonsModel::PresenceDisplayRole);
        case Qt::DisplayRole:
            return mapToSource(proxyIndex).data(Qt::DisplayRole);
        case KTp::RowTypeRole:
            //if the person has max 1 child, it's a fake person, so treat it as contact row
            if (mapToSource(proxyIndex).parent().isValid() || sourceModel()->rowCount(mapToSource(proxyIndex)) <= 1) {
                return KTp::ContactRowType;
            } else {
                return KTp::PersonRowType;
            }
        case KTp::ContactAvatarPathRole:
            return mapToSource(proxyIndex).data(PersonsModel::PhotosRole);
        case KTp::ContactAvatarPixmapRole:
            return contactPixmap(proxyIndex);
        case KTp::IdRole:
            return mapToSource(proxyIndex).data(PersonsModel::IMsRole);
        case KTp::HeaderTotalUsersRole:
            return sourceModel()->rowCount(mapToSource(proxyIndex));
        case KTp::ContactGroupsRole:
            return mapToSource(proxyIndex).data(PersonsModel::GroupsRole);
        case KTp::NepomukUriRole:
            return mapToSource(proxyIndex).data(PersonsModel::UriRole);
    }

    int j = sourceModel()->rowCount(mapToSource(proxyIndex));
    

    KTp::ContactPtr contact;

    if (j > 0) {
        KTp::ContactPtr mostOnlineContact;

        Q_FOREACH(const QVariant &v, mapToSource(proxyIndex).data(PersonsModel::IMsRole).toList()) {
            KTp::ContactPtr c = imPlugin->contactForContactId(v.toString());
            if (mostOnlineContact.isNull() && !c.isNull()) {
                mostOnlineContact = c;
                continue;
            }
            if (!c.isNull()) {
                if (c->presence() < mostOnlineContact->presence()) {
                    mostOnlineContact = c;
                }
            }
        }
        contact = mostOnlineContact;
    } else if (j == 0) {
        contact = imPlugin->contactForContactId(mapToSource(proxyIndex).data(PersonsModel::IMsRole).toString());
    }

    if (!contact.isNull()) {
        switch (role) {
            case KTp::AccountRole:
                return QVariant::fromValue<Tp::AccountPtr>(imPlugin->accountForContact(contact));
                break;
            case KTp::ContactRole:
                return QVariant::fromValue<KTp::ContactPtr>(contact);
                break;
            case KTp::ContactPresenceMessageRole:
                return contact->presence().statusMessage();
                break;
            case KTp::ContactIsBlockedRole:
                return contact->isBlocked();
                break;
            case KTp::ContactCanTextChatRole:
                return true;
                break;
            case KTp::ContactCanAudioCallRole:
                return contact->audioCallCapability();
                break;
            case KTp::ContactCanVideoCallRole:
                return contact->videoCallCapability();
                break;
            case KTp::ContactCanFileTransferRole:
                return contact->fileTransferCapability();
                break;
            case KTp::ContactClientTypesRole:
                return contact->clientTypes();
                break;
        }
    } else if (contact.isNull() && role == KTp::AccountRole) {
        QVariant accountPath = mapToSource(proxyIndex).data(PersonsModel::UserRole);
        if (accountPath.type() == QVariant::List) {
            return QVariant::fromValue<Tp::AccountPtr>(imPlugin->accountManager()->accountForObjectPath(accountPath.toList().first().toString()));
        } else {
            return QVariant::fromValue<Tp::AccountPtr>(imPlugin->accountManager()->accountForObjectPath(accountPath.toString()));
        }
    }
//     }

    return mapToSource(proxyIndex).data(role);
}

QVariant KPeopleTranslationProxy::translatePresence(const QVariant &presenceName) const
{
    if (presenceName == QLatin1String("available")) {
        return Tp::ConnectionPresenceTypeAvailable;
    }

    if (presenceName == QLatin1String("away")) {
        return Tp::ConnectionPresenceTypeAway;
    }

    if (presenceName == QLatin1String("busy") || presenceName == QLatin1String("dnd")) {
        return Tp::ConnectionPresenceTypeBusy;
    }

    if (presenceName == QLatin1String("xa")) {
        return Tp::ConnectionPresenceTypeExtendedAway;
    }

    if (presenceName == QLatin1String("hidden")) {
        return Tp::ConnectionPresenceTypeHidden;
    }

    return Tp::ConnectionPresenceTypeOffline;
}

QPixmap KPeopleTranslationProxy::contactPixmap(const QModelIndex &index) const
{
    QPixmap avatar;

    int presenceType = index.data(KTp::ContactPresenceTypeRole).toInt();
    //we need some ID to generate proper cache key for this contact
    //so we'll use the contact's uri
    const QString id = index.data(KTp::NepomukUriRole).toString();

    //key for the pixmap cache, so we can look up the avatar
    const QString keyCache = id + (presenceType == Tp::ConnectionPresenceTypeOffline ? QLatin1String("-offline") : QLatin1String("-online"));

    //check pixmap cache for the avatar, if not present, load the avatar
    if (!QPixmapCache::find(keyCache, avatar)){
        const QVariantList files = index.data(KTp::ContactAvatarPathRole).toList();
        QString file;
        if (!files.isEmpty()) {
            file = files.first().toUrl().toLocalFile();
        }

        //QPixmap::load() checks for empty path
        avatar.load(file);

        //if the above didn't succeed, we need to load the generic icon
        if (avatar.isNull()) {
            avatar = KIconLoader::global()->loadIcon(QLatin1String("im-user"), KIconLoader::NoGroup, 96);
        }

        //if the contact is offline, gray it out
        if (presenceType == Tp::ConnectionPresenceTypeOffline) {
            QImage image = avatar.toImage();
            const QPixmap alpha = avatar.alphaChannel();
            for (int i = 0; i < image.width(); ++i) {
                for (int j = 0; j < image.height(); ++j) {
                    int colour = qGray(image.pixel(i, j));
                    image.setPixel(i, j, qRgb(colour, colour, colour));
                }
            }
            avatar = avatar.fromImage(image);
            avatar.setAlphaChannel(alpha);
        }

        //insert the contact into pixmap cache for faster lookup
        QPixmapCache::insert(keyCache, avatar);
    }

    return avatar;
}
