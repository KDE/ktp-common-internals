/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2011 Francesco Nwokeka <francesco.nwokeka@gmail.com>
 * Copyright (C) 2012 Dominik Cermak <d.cermak@arcor.de>
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

#include "join-chat-room-dialog.h"
#include "ui_join-chat-room-dialog.h"

#include <KTp/Models/rooms-model.h>

#include <KConfig>
#include <KDebug>
#include <KInputDialog>
#include <KMessageBox>
#include <KNotification>
#include <KPushButton>
#include <KCompletionBox>

#include <TelepathyQt/AccountSet>
#include <TelepathyQt/AccountCapabilityFilter>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountPropertyFilter>
#include <TelepathyQt/AndFilter>
#include <TelepathyQt/ChannelTypeRoomListInterface>
#include <TelepathyQt/PendingChannel>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/RequestableChannelClassSpec>
#include <TelepathyQt/RoomListChannel>

#include <QSortFilterProxyModel>

KTp::JoinChatRoomDialog::JoinChatRoomDialog(Tp::AccountManagerPtr accountManager, QWidget* parent)
    : KDialog(parent, Qt::Dialog)
    , ui(new Ui::JoinChatRoomDialog)
    , m_model(new RoomsModel(this))
    , m_favoritesModel(new FavoriteRoomsModel(this))
    , m_favoritesProxyModel(new QSortFilterProxyModel(this))
    , m_recentComp(new KCompletion)
{
    QWidget *joinChatRoomDialog = new QWidget(this);
    ui->setupUi(joinChatRoomDialog);
    setMainWidget(joinChatRoomDialog);
    setWindowIcon(KIcon(QLatin1String("telepathy-kde")));

    // config
    KSharedConfigPtr commonConfig = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    m_favoriteRoomsGroup = commonConfig->group(QLatin1String("FavoriteRooms"));
    m_recentRoomsGroup = commonConfig->group(QLatin1String("RecentChatRooms"));

    Q_FOREACH (const QString &key, m_recentRoomsGroup.keyList()) {
        if (!m_recentRoomsGroup.readEntry(key, QStringList()).isEmpty()) {
            m_recentRooms.insert(key, m_recentRoomsGroup.readEntry(key, QStringList()));
        }
    }

    loadFavoriteRooms();

    // disable OK button on start
    button(Ok)->setEnabled(false);

    //set icons
    ui->addFavoritePushButton->setIcon(KIcon(QLatin1String("list-add")));
    ui->removeFavoritePushButton->setIcon(KIcon(QLatin1String("list-remove")));
    ui->removeRecentPushButton->setIcon(KIcon(QLatin1String("list-remove")));
    ui->clearRecentPushButton->setIcon(KIcon(QLatin1String("edit-clear-list")));

    Tp::AccountPropertyFilterPtr isOnlineFilter = Tp::AccountPropertyFilter::create();
    isOnlineFilter->addProperty(QLatin1String("online"), true);

    Tp::AccountCapabilityFilterPtr capabilityFilter = Tp::AccountCapabilityFilter::create(
                Tp::RequestableChannelClassSpecList() << Tp::RequestableChannelClassSpec::textChatroom());

    Tp::AccountFilterPtr filter = Tp::AndFilter<Tp::Account>::create((QList<Tp::AccountFilterConstPtr>() <<
                                             isOnlineFilter <<
                                             capabilityFilter));

    ui->comboBox->setAccountSet(accountManager->filterAccounts(filter));

    // apply the filter after populating
    onAccountSelectionChanged(ui->comboBox->currentIndex());

    // favoritesTab
    m_favoritesProxyModel->setSourceModel(m_favoritesModel);
    m_favoritesProxyModel->setFilterKeyColumn(FavoriteRoomsModel::AccountIdentifierColumn);
    m_favoritesProxyModel->setDynamicSortFilter(true);

    ui->listView->setModel(m_favoritesProxyModel);
    ui->listView->setModelColumn(FavoriteRoomsModel::NameColumn);

    // recentTab
    m_recentComp->setCompletionMode(KGlobalSettings::CompletionPopup);
    m_recentComp->setIgnoreCase(true);

    ui->lineEdit->setCompletionObject(m_recentComp);
    ui->lineEdit->setAutoDeleteCompletionObject(true);

    // queryTab
    if (ui->comboBox->count() > 0) {
        ui->queryPushButton->setEnabled(true);
    }

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_model);
    proxyModel->setSortLocaleAware(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(RoomsModel::NameColumn);
    proxyModel->setDynamicSortFilter(true);

    ui->treeView->setModel(proxyModel);
    ui->treeView->header()->setResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->sortByColumn(RoomsModel::NameColumn, Qt::AscendingOrder);

    // connects
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));
    connect(ui->listView, SIGNAL(clicked(QModelIndex)), this, SLOT(onFavoriteRoomClicked(QModelIndex)));
    connect(ui->addFavoritePushButton, SIGNAL(clicked(bool)), this, SLOT(addFavorite()));
    connect(ui->removeFavoritePushButton, SIGNAL(clicked(bool)), this, SLOT(removeFavorite()));
    connect(ui->recentListWidget, SIGNAL(currentTextChanged(QString)), ui->lineEdit, SLOT(setText(QString)));
    connect(ui->recentListWidget, SIGNAL(currentTextChanged(QString)), this, SLOT(onRecentRoomClicked()));
    connect(ui->removeRecentPushButton, SIGNAL(clicked(bool)), this , SLOT(removeRecentRoom()));
    connect(ui->clearRecentPushButton, SIGNAL(clicked(bool)), this, SLOT(clearRecentRooms()));
    connect(ui->queryPushButton, SIGNAL(clicked(bool)), this, SLOT(getRoomList()));
    connect(ui->stopPushButton, SIGNAL(clicked(bool)), this, SLOT(stopListing()));
    connect(ui->treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(onRoomClicked(QModelIndex)));
    connect(ui->filterBar, SIGNAL(textChanged(QString)), proxyModel, SLOT(setFilterFixedString(QString)));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAccountSelectionChanged(int)));
    connect(button(Ok), SIGNAL(clicked(bool)), this, SLOT(addRecentRoom()));
}

KTp::JoinChatRoomDialog::~JoinChatRoomDialog()
{
    delete ui;
}

Tp::AccountPtr KTp::JoinChatRoomDialog::selectedAccount() const
{
    return ui->comboBox->currentAccount();
}

void KTp::JoinChatRoomDialog::onAccountSelectionChanged(int newIndex)
{
    // Show only favorites associated with the selected account
    Q_UNUSED(newIndex)

    if (!ui->comboBox->currentAccount()) {
        return;
    }

    QString accountIdentifier = ui->comboBox->currentAccount()->uniqueIdentifier();
    m_favoritesProxyModel->setFilterFixedString(accountIdentifier);

    // Provide only rooms recently used with the selected account for completion
    m_recentComp->clear();

    Q_FOREACH (const QString &room, m_recentRooms.value(accountIdentifier)) {
        m_recentComp->addItem(room);
    }

    // Show only rooms recently used with the selected account in the list
    ui->recentListWidget->clear();
    ui->recentListWidget->addItems(m_recentRooms.value(accountIdentifier));

    // Enable/disable the buttons as appropriate
    bool recentListNonEmpty = m_recentRooms.value(accountIdentifier).size() > 0;

    ui->clearRecentPushButton->setEnabled(recentListNonEmpty);
    ui->removeRecentPushButton->setEnabled(recentListNonEmpty);
}

void KTp::JoinChatRoomDialog::addFavorite()
{
    if (!ui->comboBox->currentAccount()) {
        return;
    }

    bool ok = false;
    QString favoriteHandle = ui->lineEdit->text();
    QString favoriteAccount = ui->comboBox->currentAccount()->uniqueIdentifier();

    if (m_favoritesModel->containsRoom(favoriteHandle, favoriteAccount)) {
        KMessageBox::sorry(this, i18n("This room is already in your favorites."));
    } else {
        QString favoriteName = KInputDialog::getText(i18n("Add room"), i18n("Name"), QString(), &ok);

        if (ok) {
            QString key = favoriteHandle + favoriteAccount;

            // Write it to the config file
            QVariantList favorite;
            favorite.append(favoriteName);
            favorite.append(favoriteHandle);
            favorite.append(favoriteAccount);
            m_favoriteRoomsGroup.writeEntry(key, favorite);
            m_favoriteRoomsGroup.sync();

            // Insert it into the model
            QVariantMap room;
            room.insert(QLatin1String("name"), favoriteName);
            room.insert(QLatin1String("handle-name"), favoriteHandle);
            room.insert(QLatin1String("account-identifier"), favoriteAccount);
            m_favoritesModel->addRoom(room);
        }
    }
}

void KTp::JoinChatRoomDialog::removeFavorite()
{
    QString favoriteHandle = ui->listView->currentIndex().data(FavoriteRoomsModel::HandleNameRole).toString();
    QString favoriteAccount = ui->comboBox->currentAccount()->uniqueIdentifier();
    QVariantMap room = ui->listView->currentIndex().data(FavoriteRoomsModel::FavoriteRoomRole).value<QVariantMap>();

    QString key = favoriteHandle + favoriteAccount;

    if(m_favoriteRoomsGroup.keyList().contains(key)) {
        m_favoriteRoomsGroup.deleteEntry(key);
        m_favoriteRoomsGroup.sync();
        m_favoritesModel->removeRoom(room);

        if (m_favoritesModel->countForAccount(favoriteAccount) == 0) {
            ui->removeFavoritePushButton->setEnabled(false);
        }
    }
}

void KTp::JoinChatRoomDialog::addRecentRoom()
{
    Tp::AccountPtr account = ui->comboBox->currentAccount();
    if (!account) {
        return;
    }

    QString accountIdentifier = account->uniqueIdentifier();
    QString handle = ui->lineEdit->text();

    if (!handle.isEmpty()) {
        if (m_recentRooms.contains(accountIdentifier)) {
            QStringList currentList = m_recentRooms.take(accountIdentifier);

            // If the handle is already in the list move it at the first position
            // because now it is the most recent
            if (currentList.contains(handle)) {
                currentList.move(currentList.indexOf(handle), 0);
                m_recentRooms.insert(accountIdentifier, currentList);
            // else just insert it at the first position and check for the size
            } else {
                currentList.insert(0, handle);

                if (currentList.size() > 8) {
                    currentList.removeLast();
                }

                m_recentRooms.insert(accountIdentifier, currentList);
            }
        } else {
            m_recentRooms.insert(accountIdentifier, QStringList(handle));
        }

        // Write it to the config
        m_recentRoomsGroup.writeEntry(accountIdentifier, m_recentRooms.value(accountIdentifier));
        m_recentRoomsGroup.sync();
    }
}

void KTp::JoinChatRoomDialog::removeRecentRoom()
{
    QString accountIdentifier = ui->comboBox->currentAccount()->uniqueIdentifier();
    QString handle = ui->recentListWidget->currentItem()->text();

    // Remove the entry
    QStringList currentList = m_recentRooms.take(accountIdentifier);
    currentList.removeOne(handle);
    m_recentRooms.insert(accountIdentifier, currentList);

    // Update the completion and list
    onAccountSelectionChanged(ui->comboBox->currentIndex());

    // Write it to the config
    m_recentRoomsGroup.writeEntry(accountIdentifier, m_recentRooms.value(accountIdentifier));
    m_recentRoomsGroup.sync();

    // Disable the button
    ui->removeRecentPushButton->setEnabled(false);
}

void KTp::JoinChatRoomDialog::clearRecentRooms()
{
    QString accountIdentifier = ui->comboBox->currentAccount()->uniqueIdentifier();

    // Remove all entries for the current account
    m_recentRooms.remove(accountIdentifier);

    // Update the completion and list
    onAccountSelectionChanged(ui->comboBox->currentIndex());

    // Update the config
    m_recentRoomsGroup.deleteEntry(accountIdentifier);
    m_recentRoomsGroup.sync();
}

void KTp::JoinChatRoomDialog::getRoomList()
{
    Tp::AccountPtr account = ui->comboBox->currentAccount();
    if (!account) {
        return;
    }

    // Clear the list from previous items
    m_model->clearRoomInfoList();

    // Build the channelrequest
    QVariantMap request;
    request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".ChannelType"),
                   TP_QT_IFACE_CHANNEL_TYPE_ROOM_LIST);
    request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType"),
                   Tp::HandleTypeNone);

    // If the user provided a server use it, else use the standard server for the selected account
    if (!ui->serverLineEdit->text().isEmpty()) {
        request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".Type.RoomList.Server"),
                       ui->serverLineEdit->text());
    }

    m_pendingRoomListChannel = account->createAndHandleChannel(request, QDateTime::currentDateTime());
    connect(m_pendingRoomListChannel, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onRoomListChannelReadyForHandling(Tp::PendingOperation*)));

}

void KTp::JoinChatRoomDialog::stopListing()
{
    m_iface->StopListing();
}

void KTp::JoinChatRoomDialog::onRoomListChannelReadyForHandling(Tp::PendingOperation *operation)
{
    if (operation->isError()) {
        kDebug() << operation->errorName();
        kDebug() << operation->errorMessage();
        QString errorMsg(operation->errorName() + QLatin1String(": ") + operation->errorMessage());
        sendNotificationToUser(errorMsg);
    } else {
        m_roomListChannel = m_pendingRoomListChannel->channel();

        connect(m_roomListChannel->becomeReady(),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onRoomListChannelReady(Tp::PendingOperation*)));
    }
}

void KTp::JoinChatRoomDialog::onRoomListChannelReady(Tp::PendingOperation *operation)
{
    if (operation->isError()) {
        kDebug() << operation->errorName();
        kDebug() << operation->errorMessage();
        QString errorMsg(operation->errorName() + QLatin1String(": ") + operation->errorMessage());
        sendNotificationToUser(errorMsg);
    } else {
        m_iface = m_roomListChannel->interface<Tp::Client::ChannelTypeRoomListInterface>();

        m_iface->ListRooms();

        connect(m_iface, SIGNAL(ListingRooms(bool)), SLOT(onListing(bool)));
        connect(m_iface, SIGNAL(GotRooms(Tp::RoomInfoList)), SLOT(onGotRooms(Tp::RoomInfoList)));
    }
}

void KTp::JoinChatRoomDialog::onRoomListChannelClosed(Tp::PendingOperation *operation)
{
    if (operation->isError()) {
        kDebug() << operation->errorName();
        kDebug() << operation->errorMessage();
        QString errorMsg(operation->errorName() + QLatin1String(": ") + operation->errorMessage());
        sendNotificationToUser(errorMsg);
    } else {
        ui->queryPushButton->setEnabled(true);
        ui->stopPushButton->setEnabled(false);
    }
}

void KTp::JoinChatRoomDialog::onListing(bool isListing)
{
    if (isListing) {
        kDebug() << "listing";
        ui->queryPushButton->setEnabled(false);
        ui->stopPushButton->setEnabled(true);
    } else {
        kDebug() << "finished listing";
        Tp::PendingOperation *op =  m_roomListChannel->requestClose();
        connect(op,
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onRoomListChannelClosed(Tp::PendingOperation*)));
    }
}

void KTp::JoinChatRoomDialog::onGotRooms(Tp::RoomInfoList roomInfoList)
{
    m_model->addRooms(roomInfoList);
}

void KTp::JoinChatRoomDialog::onFavoriteRoomClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        ui->removeFavoritePushButton->setEnabled(true);
        ui->lineEdit->setText(index.data(FavoriteRoomsModel::HandleNameRole).toString());
    } else {
        ui->removeFavoritePushButton->setEnabled(false);
    }
}

void KTp::JoinChatRoomDialog::onRecentRoomClicked()
{
    ui->removeRecentPushButton->setEnabled(true);
}

void KTp::JoinChatRoomDialog::onRoomClicked(const QModelIndex& index)
{
    ui->lineEdit->setText(index.data(RoomsModel::HandleNameRole).toString());
}

QString KTp::JoinChatRoomDialog::selectedChatRoom() const
{
    return ui->lineEdit->text();
}

void KTp::JoinChatRoomDialog::onTextChanged(QString newText)
{
    if (newText.isEmpty()) {
        button(Ok)->setEnabled(false);
        ui->addFavoritePushButton->setEnabled(false);
    } else {
        button(Ok)->setEnabled(true);
        ui->addFavoritePushButton->setEnabled(true);
    }
}

void KTp::JoinChatRoomDialog::sendNotificationToUser(const QString& errorMsg)
{
    //The pointer is automatically deleted when the event is closed
    KNotification *notification;
    notification = new KNotification(QLatin1String("telepathyError"), this);

    notification->setText(errorMsg);
    notification->sendEvent();
}

void KTp::JoinChatRoomDialog::loadFavoriteRooms()
{
    QList<QVariantMap> roomList;

    Q_FOREACH(const QString &key, m_favoriteRoomsGroup.keyList()) {
        QVariantList favorite = m_favoriteRoomsGroup.readEntry(key, QVariantList());
        QString favoriteName = favorite.at(0).toString();
        QString favoriteHandle = favorite.at(1).toString();
        QString favoriteAccount = favorite.at(2).toString();

        QVariantMap room;
        room.insert(QLatin1String("name"), favoriteName);
        room.insert(QLatin1String("handle-name"), favoriteHandle);
        room.insert(QLatin1String("account-identifier"), favoriteAccount);
        roomList.append(room);
    }

    m_favoritesModel->addRooms(roomList);
}
