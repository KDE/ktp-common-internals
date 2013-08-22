/*
 * Model of all accounts with inbuilt grouping and filtering
 *
 * Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
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

#include "contacts-model.h"

#include "contacts-list-model.h"
#include "accounts-tree-proxy-model.h"
#include "groups-tree-proxy-model.h"
#include "text-channel-watcher-proxy-model.h"

#include <TelepathyQt/ClientRegistrar>

#ifdef HAVE_KPEOPLE
#include <Nepomuk2/ResourceManager>
#include <KPeople/PersonsModel>
#include <kpeople/personsmodelfeature.h>
#include "kpeopletranslationproxy.h"
#endif

#include <KDebug>


namespace KTp
{
class ContactsModel::Private
{
public:
    GroupMode groupMode;
    bool trackUnread;
    bool nepomukEnabled;
    QWeakPointer<KTp::AbstractGroupingProxyModel> proxy;
    QAbstractItemModel *source;
    Tp::AccountManagerPtr accountManager;
    Tp::ClientRegistrarPtr clientRegistrar;
    Tp::SharedPtr<KTp::TextChannelWatcherProxyModel> channelWatcherProxy;
};
}


KTp::ContactsModel::ContactsModel(QObject *parent)
    : KTp::ContactsFilterModel(parent),
      d(new Private)
{
    d->groupMode = NoGrouping;
    d->trackUnread = false;
    d->nepomukEnabled = false;
#ifdef HAVE_KPEOPLE
    d->nepomukEnabled = Nepomuk2::ResourceManager::instance()->initialized();

    if (d->nepomukEnabled) {
        kDebug() << "Nepomuk is enabled, using kpeople model";
        KPeople::PersonsModel *personsModel = new KPeople::PersonsModel(this);

        KPeople::PersonsModelFeature accountFeature;
        QHash<QString, int> bindingMap;
        bindingMap[QLatin1String("account")] = KPeople::PersonsModel::UserRole;
        accountFeature.setBindingsMap(bindingMap);
        accountFeature.setOptional(false);
        accountFeature.setQueryPart(QLatin1String("?uri nco:hasIMAccount ?imAccount . ?imAccount nco:isAccessedBy ?accessedBy . ?accessedBy telepathy:accountIdentifier ?account . "));

        personsModel->startQuery(QList<KPeople::PersonsModelFeature>() << KPeople::PersonsModelFeature::imModelFeature(KPeople::PersonsModelFeature::Mandatory)
                                                            << accountFeature
                                                            << KPeople::PersonsModelFeature::avatarModelFeature()
                                                            << KPeople::PersonsModelFeature::groupsModelFeature()
                                                            << KPeople::PersonsModelFeature::fullNameModelFeature()
                                                            << KPeople::PersonsModelFeature::nicknameModelFeature());
        d->source = new KPeopleTranslationProxy(this);
        qobject_cast<KPeopleTranslationProxy*>(d->source)->setSourceModel(personsModel);
    } else
#endif
    {
        kDebug() << "Nepomuk is disabled, using normal model";
        d->source = new KTp::ContactsListModel(this);
    }
}

KTp::ContactsModel::~ContactsModel()
{
    delete d;
}


void KTp::ContactsModel::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    d->accountManager = accountManager;

    updateGroupProxyModels();

    //set the account manager after we've reloaded the groups so that we don't send a list to the view, only to replace it with a grouped tree
#ifdef HAVE_KPEOPLE
    if (!d->nepomukEnabled) {
        qobject_cast<ContactsListModel*>(d->source)->setAccountManager(accountManager);
    }
#else
    qobject_cast<ContactsListModel*>(d->source)->setAccountManager(accountManager);
#endif
}

Tp::AccountManagerPtr KTp::ContactsModel::accountManager() const
{
    return d->accountManager;
}

void KTp::ContactsModel::setGroupMode(KTp::ContactsModel::GroupMode mode)
{
    if (mode == d->groupMode) {
        //if nothing has changed, do nothing.
        return;
    }

    d->groupMode = mode;

    updateGroupProxyModels();

    Q_EMIT groupModeChanged();
}

KTp::ContactsModel::GroupMode KTp::ContactsModel::groupMode() const
{
    return d->groupMode;
}

void KTp::ContactsModel::setTrackUnreadMessages(bool trackUnread)
{
    if (d->trackUnread == trackUnread) {
        return;
    }
    d->trackUnread = trackUnread;

    updateGroupProxyModels();

    Q_EMIT trackUnreadMessagesChanged();
}

bool KTp::ContactsModel::trackUnreadMessages() const
{
    return d->trackUnread;
}

void KTp::ContactsModel::updateGroupProxyModels()
{
    //reset the filter
    //trying to track current selections whilst updating proxy models can cause issues
    //debug versions of Qt will assert
    reset();

    //if there no account manager there's not a lot point doing anything
    if (!d->accountManager) {
        return;
    }

    //if needed set up the client registrar and observer proxy model
    if (d->trackUnread && d->clientRegistrar.isNull()) {
        d->clientRegistrar = Tp::ClientRegistrar::create(d->accountManager);
        d->channelWatcherProxy = Tp::SharedPtr<KTp::TextChannelWatcherProxyModel>(new TextChannelWatcherProxyModel());
        d->channelWatcherProxy->setSourceModel(d->source);
        d->clientRegistrar->registerClient(Tp::AbstractClientPtr::dynamicCast(d->channelWatcherProxy), QLatin1String("ListWatcher"));
    } else if (!d->trackUnread) {
        //delete the client registrar
        d->clientRegistrar.reset();
        d->channelWatcherProxy.reset();
    }

    QAbstractItemModel *modelToGroup = 0;
    if (d->trackUnread) {
        modelToGroup = d->channelWatcherProxy.data();
    } else {
        modelToGroup = d->source;
    }

    //delete any previous proxy
    if (d->proxy) {
        d->proxy.data()->deleteLater();
    }

    switch (d->groupMode) {
    case NoGrouping:
        //This is a workaround to a Qt assert which gets confused when we switch from a source model that was
        //part of the proxy chain, and is now used in the view directly
        //
        //do not disable until you have tested on Qt in debug mode
        setSourceModel(0);
        setSourceModel(modelToGroup);
        break;
    case AccountGrouping:
        d->proxy = new KTp::AccountsTreeProxyModel(modelToGroup, d->accountManager);
        setSourceModel(d->proxy.data());
        break;
    case GroupGrouping:
        d->proxy = new KTp::GroupsTreeProxyModel(modelToGroup);
        setSourceModel(d->proxy.data());
        break;
    }
}

void KTp::ContactsModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    KTp::ContactsFilterModel::setSourceModel(sourceModel);

    //Qt automatically updates the role names to use that of the source model
    //this causes problems when we have multiple source models that we change between
    //instead we update here just after we set a source model

    //in Qt5.0 override the virtual roleNames() method and do it there.

    QHash<int, QByteArray> roles = roleNames();
    roles[KTp::RowTypeRole]= "type";
    roles[KTp::IdRole]= "id";

    roles[KTp::ContactRole]= "contact";
    roles[KTp::AccountRole]= "account";

    roles[KTp::ContactClientTypesRole]= "clientTypes";
    roles[KTp::ContactAvatarPathRole]= "avatar";
    roles[KTp::ContactAvatarPixmapRole]="avatarPixmap";
    roles[KTp::ContactGroupsRole]= "groups";
    roles[KTp::ContactPresenceNameRole]= "presenceName";
    roles[KTp::ContactPresenceMessageRole]= "presenceMessage";
    roles[KTp::ContactPresenceTypeRole]= "presenceType";
    roles[KTp::ContactPresenceIconRole]= "presenceIcon";
    roles[KTp::ContactSubscriptionStateRole]= "subscriptionState";
    roles[KTp::ContactPublishStateRole]= "publishState";
    roles[KTp::ContactIsBlockedRole]= "blocked";
    roles[KTp::ContactHasTextChannelRole]= "hasTextChannel";
    roles[KTp::ContactUnreadMessageCountRole]= "unreadMessageCount";
    roles[KTp::ContactLastMessageRole]= "lastMessage";
    roles[KTp::ContactLastMessageDirectionRole]= "lastMessageDirection";
    roles[KTp::ContactCanTextChatRole]= "textChat";
    roles[KTp::ContactCanFileTransferRole]= "fileTransfer";
    roles[KTp::ContactCanAudioCallRole]= "audioCall";
    roles[KTp::ContactCanVideoCallRole]= "videoCall";
    roles[KTp::ContactTubesRole]= "tubes";
    setRoleNames(roles);
}
