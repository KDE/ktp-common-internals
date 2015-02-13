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
#include "KTp/types.h"
#include "KTp/global-contact-manager.h"

#include <KPeople/PersonsModel>
#include <KPeople/KPeopleBackend/AbstractContact>

#include <KIconLoader>

#include <QPixmapCache>

using namespace KPeople;


KPeopleTranslationProxy::KPeopleTranslationProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

KPeopleTranslationProxy::~KPeopleTranslationProxy()
{
}

QVariant KPeopleTranslationProxy::data(const QModelIndex &proxyIndex, int role) const
{
    if (!proxyIndex.isValid()) {
        return QVariant();
    }

//     IMPersonsDataSource *imPlugin = qobject_cast<IMPersonsDataSource*>(PersonPluginManager::presencePlugin());
//
//     if (!imPlugin) {
//         qCWarning(KTP_MODELS) << "No imPlugin";
//         return QVariant();
//     }
//
    const QModelIndex sourceIndex = mapToSource(proxyIndex);
    AbstractContact::Ptr contact = sourceIndex.data(KPeople::PersonsModel::PersonVCardRole).value<AbstractContact::Ptr>();

    switch (role) {
        case KTp::ContactPresenceTypeRole:
            return s_presenceStrings.key(contact->customProperty(S_KPEOPLE_PROPERTY_PRESENCE).toString());
        case KTp::ContactPresenceIconRole:
            return KPeople::iconNameForPresenceString(contact->customProperty(S_KPEOPLE_PROPERTY_PRESENCE).toString());
//         case KTp::ContactPresenceNameRole:
//             return sourceIndex.data(PersonsModel::PresenceDisplayRole);
        case Qt::DisplayRole:
            return sourceIndex.data(KPeople::PersonsModel::FormattedNameRole);
        case KTp::RowTypeRole:
            if (proxyIndex.parent().isValid() || sourceModel()->rowCount(sourceIndex) <= 1) {
                return KTp::ContactRowType;
            } else {
                return KTp::PersonRowType;
            }
//             //if the person has max 1 child, it's a fake person, so treat it as contact row
//             if (sourceIndex.parent().isValid() || sourceModel()->rowCount(sourceIndex) <= 1) {
//                 return KTp::ContactRowType;
//             } else {
//                 return KTp::PersonRowType;
//             }
//         case KTp::ContactAvatarPathRole:
//             return sourceIndex.data(PersonsModel::PhotosRole);
        case KTp::ContactAvatarPixmapRole:
            return sourceIndex.data(KPeople::PersonsModel::PhotoRole);
        case KTp::ContactUriRole:
            return contact->customProperty(S_KPEOPLE_PROPERTY_CONTACT_URI);
        case KTp::IdRole:
            return contact->customProperty(S_KPEOPLE_PROPERTY_CONTACT_ID);
        case KTp::ContactIsBlockedRole:
            return contact->customProperty(S_KPEOPLE_PROPERTY_IS_BLOCKED);
//         case KTp::HeaderTotalUsersRole:
//             return sourceModel()->rowCount(sourceIndex);
        case KTp::ContactGroupsRole:
            return sourceIndex.data(PersonsModel::GroupsRole);
        case KTp::PersonIdRole:
            return sourceIndex.data(PersonsModel::PersonUriRole);
        case KTp::ContactVCardRole:
            return sourceIndex.data(KPeople::PersonsModel::PersonVCardRole);
    }

    AbstractContact::List contacts = sourceIndex.data(PersonsModel::ContactsVCardRole).value<AbstractContact::List>();

    int mostOnlineIndex = 0;

    for (int i = 0; i < contacts.size(); i++) {
        if (KPeople::presenceSortPriority(contact->customProperty(S_KPEOPLE_PROPERTY_PRESENCE).toString())
            < KPeople::presenceSortPriority(contacts.at(mostOnlineIndex)->customProperty(S_KPEOPLE_PROPERTY_PRESENCE).toString())) {

            mostOnlineIndex = i;
        }
    }

    AbstractContact::Ptr informationContact = contacts.isEmpty() ? contact : contacts.at(mostOnlineIndex);
    QVariant rValue = dataForKTpContact(informationContact->customProperty(S_KPEOPLE_PROPERTY_ACCOUNT_PATH).toString(),
                               informationContact->customProperty(S_KPEOPLE_PROPERTY_CONTACT_ID).toString(),
                               role);

    if (rValue.isNull()) {
        return sourceIndex.data(role);
    } else {
        return rValue;
    }
}

QVariant KPeopleTranslationProxy::dataForKTpContact(const QString &accountPath, const QString &contactId, int role) const
{
    if (accountPath.isEmpty()) {
        return QVariant();
    }
    if (role == KTp::AccountRole) {
        return QVariant::fromValue<Tp::AccountPtr>(KTp::contactManager()->accountForAccountPath(accountPath));
    }

    KTp::ContactPtr ktpContact = KTp::contactManager()->contactForContactId(accountPath, contactId);
    if (!ktpContact.isNull()) {
        switch (role) {
        case KTp::ContactRole:
            return QVariant::fromValue<KTp::ContactPtr>(ktpContact);
            break;
        case KTp::ContactPresenceMessageRole:
            return ktpContact->presence().statusMessage();
            break;
        case KTp::ContactIsBlockedRole:
            return ktpContact->isBlocked();
            break;
        case KTp::ContactCanTextChatRole:
            return true;
            break;
        case KTp::ContactCanAudioCallRole:
            return ktpContact->audioCallCapability();
            break;
        case KTp::ContactCanVideoCallRole:
            return ktpContact->videoCallCapability();
            break;
        case KTp::ContactCanFileTransferRole:
            return ktpContact->fileTransferCapability();
            break;
        case KTp::ContactClientTypesRole:
            return ktpContact->clientTypes();
            break;
        }
    }

    return QVariant();
}

bool KPeopleTranslationProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);

    //if no valid presence (not even "offline") .. reject the contact
    return !sourceIndex.data(KPeople::PersonsModel::PersonVCardRole).value<KPeople::AbstractContact::Ptr>()->customProperty(S_KPEOPLE_PROPERTY_PRESENCE).isNull();
}

QVariant KPeopleTranslationProxy::translatePresence(const QVariant &presenceName) const
{


    return Tp::ConnectionPresenceTypeOffline;
}

QPixmap KPeopleTranslationProxy::contactPixmap(const QModelIndex &index) const
{
    QPixmap avatar;

    int presenceType = index.data(KTp::ContactPresenceTypeRole).toInt();
    //we need some ID to generate proper cache key for this contact
    //so we'll use the contact's uri
    const QString id = index.data(KTp::IdRole).toString();

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
            const QImage alpha = image.alphaChannel();
            for (int i = 0; i < image.width(); ++i) {
                for (int j = 0; j < image.height(); ++j) {
                    int colour = qGray(image.pixel(i, j));
                    image.setPixel(i, j, qRgb(colour, colour, colour));
                }
            }
            image.setAlphaChannel(alpha);
            avatar = avatar.fromImage(image);
        }

        //insert the contact into pixmap cache for faster lookup
        QPixmapCache::insert(keyCache, avatar);
    }

    return avatar;
}
