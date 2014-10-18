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

#include <TelepathyQt/Account>
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
#include <TelepathyQt/PendingChannelRequest>

#include <QSortFilterProxyModel>

KTp::JoinChatRoomDialog::JoinChatRoomDialog(Tp::AccountManagerPtr accountManager, QWidget* parent)
    : KDialog(parent, Qt::Dialog)
    , ui(new Ui::JoinChatRoomDialog)
    , m_model(new RoomsModel(this))
    , m_favoritesModel(new FavoriteRoomsModel(this))
    , m_favoritesProxyModel(new QSortFilterProxyModel(this))
    , m_joinInProgress(false)
{
    QWidget *joinChatRoomDialog = new QWidget(this);
    ui->setupUi(joinChatRoomDialog);
    ui->feedbackWidget->hide();

    setMainWidget(joinChatRoomDialog);
    setWindowIcon(KIcon(QLatin1String("im-irc")));
    setWindowTitle(i18nc("Dialog title", "Join Chat Room"));

    ui->filterPicture->clear();
    ui->filterPicture->setPixmap(KIconLoader::global()->loadIcon(QLatin1String("view-filter"), KIconLoader::Small));

    // config
    KSharedConfigPtr commonConfig = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    m_favoriteRoomsGroup = commonConfig->group(QLatin1String("FavoriteRooms"));
    m_recentRoomsGroup = commonConfig->group(QLatin1String("RecentChatRooms"));

    // load favorite and recent rooms
    loadFavoriteRooms();

    // disable OK button on start
    button(Ok)->setEnabled(false);
    button(Ok)->setText(i18nc("button", "Join/Create"));
    button(Ok)->setIcon(KIcon(QLatin1String("im-irc")));
    onAccountSelectionChanged(ui->comboBox->currentIndex());
    connect(accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    // Previous Tab
    m_favoritesProxyModel->setSourceModel(m_favoritesModel);
    m_favoritesProxyModel->setFilterKeyColumn(FavoriteRoomsModel::AccountIdentifierColumn);
    m_favoritesProxyModel->setSortRole(Qt::CheckStateRole);
    m_favoritesProxyModel->setDynamicSortFilter(true);

    ui->previousView->setModel(m_favoritesProxyModel);
    ui->previousView->setHeaderHidden(true);
    ui->previousView->header()->setStretchLastSection(false);
    ui->previousView->header()->setResizeMode(FavoriteRoomsModel::BookmarkColumn, QHeaderView::ResizeToContents);
    ui->previousView->header()->setResizeMode(FavoriteRoomsModel::HandleNameColumn, QHeaderView::Stretch);
    ui->previousView->setColumnHidden(FavoriteRoomsModel::AccountIdentifierColumn, true);
    ui->previousView->sortByColumn(FavoriteRoomsModel::BookmarkColumn, Qt::DescendingOrder);


    // Search Tab
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_model);
    proxyModel->setSortLocaleAware(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(RoomsModel::NameColumn);
    proxyModel->setDynamicSortFilter(true);

    ui->queryView->setModel(proxyModel);
    ui->queryView->header()->setStretchLastSection(false);
    ui->queryView->header()->setResizeMode(0, QHeaderView::Stretch);
    ui->queryView->header()->setResizeMode(1, QHeaderView::Stretch);
    ui->queryView->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    ui->queryView->header()->setResizeMode(3, QHeaderView::ResizeToContents);
    ui->queryView->header()->setSortIndicatorShown(false);
    ui->queryView->sortByColumn(RoomsModel::NameColumn, Qt::AscendingOrder);

    // connects
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));
    connect(ui->previousView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
    connect(ui->previousView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(onFavoriteRoomSelectionChanged(QModelIndex,QModelIndex)));
    connect(m_favoritesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(onFavoriteRoomDataChanged(QModelIndex,QModelIndex)));
    connect(ui->clearRecentPushButton, SIGNAL(clicked(bool)), this, SLOT(clearRecentRooms()));
    connect(ui->serverLineEdit, SIGNAL(returnPressed()), this, SLOT(getRoomList()));
    connect(ui->queryButton, SIGNAL(clicked(bool)), this, SLOT(getRoomList()));
    connect(ui->queryView, SIGNAL(clicked(QModelIndex)), this, SLOT(onRoomClicked(QModelIndex)));
    connect(ui->queryView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
    connect(ui->filterBar, SIGNAL(textChanged(QString)), proxyModel, SLOT(setFilterFixedString(QString)));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAccountSelectionChanged(int)));
    connect(button(Ok), SIGNAL(clicked(bool)), this, SLOT(addRecentRoom()));
}

void KTp::JoinChatRoomDialog::closeEvent(QCloseEvent* e)
{
    // ignore close event if we are in the middle of an operation
    if (!m_joinInProgress) {
        KDialog::closeEvent(e);
    }
}


void KTp::JoinChatRoomDialog::onAccountManagerReady(Tp::PendingOperation* operation)
{
    Tp::AccountManagerPtr accountManager = Tp::AccountManagerPtr::qObjectCast(
        qobject_cast<Tp::PendingReady*>(operation)->proxy()
    );
    Tp::AccountPropertyFilterPtr isOnlineFilter = Tp::AccountPropertyFilter::create();
    isOnlineFilter->addProperty(QLatin1String("online"), true);

    Tp::AccountCapabilityFilterPtr capabilityFilter = Tp::AccountCapabilityFilter::create(
                Tp::RequestableChannelClassSpecList() << Tp::RequestableChannelClassSpec::textChatroom());
    Tp::AccountFilterPtr filter = Tp::AndFilter<Tp::Account>::create((QList<Tp::AccountFilterConstPtr>() <<
                                             isOnlineFilter <<
                                             capabilityFilter));

    ui->comboBox->setAccountSet(accountManager->filterAccounts(filter));

    // queryTab
    if (ui->comboBox->count() > 0) {
        ui->queryButton->setEnabled(true);
    }

    // apply the filter after populating
    onAccountSelectionChanged(ui->comboBox->currentIndex());
}

KTp::JoinChatRoomDialog::~JoinChatRoomDialog()
{
    delete ui;
}

Tp::AccountPtr KTp::JoinChatRoomDialog::selectedAccount() const
{
    return ui->comboBox->currentAccount();
}

void KTp::JoinChatRoomDialog::accept()
{
    ui->feedbackWidget->hide();
    const Tp::AccountPtr account = selectedAccount();
    if (account) {
        setJoinInProgress(true);
        Tp::PendingChannelRequest *request = account->ensureTextChatroom(selectedChatRoom());
        connect(request, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onStartChatFinished(Tp::PendingOperation*)));
    }
}


void KTp::JoinChatRoomDialog::onAccountSelectionChanged(int newIndex)
{
    // Show only favorites associated with the selected account
    Q_UNUSED(newIndex)

    if (!ui->comboBox->currentAccount()) {
        // Set a filter expression that matches no account identifier
        m_favoritesProxyModel->setFilterRegExp(QLatin1String("a^"));
        return;
    }

    QString accountIdentifier = ui->comboBox->currentAccount()->uniqueIdentifier();
    m_favoritesProxyModel->setFilterFixedString(accountIdentifier);

    // Enable/disable the buttons as appropriate
    ui->clearRecentPushButton->setEnabled(!m_recentRoomsGroup.keyList().empty());
}

void KTp::JoinChatRoomDialog::onFavoriteRoomDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // Because previousView only allows editing of a single row, assume that topLeft points to the changed data.
    Q_UNUSED(bottomRight);

    bool bookmarked = topLeft.data(Qt::CheckStateRole) == Qt::Checked ? true : false;
    const QString &favoriteHandle = topLeft.data(FavoriteRoomsModel::HandleNameRole).toString();
    const QString &favoriteAccount = topLeft.data(FavoriteRoomsModel::AccountRole).toString();

    const QString &key = favoriteHandle + favoriteAccount;

    // Write the changed room to the config file
    QVariantList favorite;
    favorite.append(favoriteHandle);
    favorite.append(favoriteAccount);

    if (bookmarked) {
        if (m_recentRoomsGroup.keyList().contains(key)) {
            m_recentRoomsGroup.deleteEntry(key);
            m_recentRoomsGroup.sync();
        }
        m_favoriteRoomsGroup.writeEntry(key, favorite);
        m_favoriteRoomsGroup.sync();
    } else {
        if (m_favoriteRoomsGroup.keyList().contains(key)) {
            m_favoriteRoomsGroup.deleteEntry(key);
            m_favoriteRoomsGroup.sync();
        }
        m_recentRoomsGroup.writeEntry(key, favorite);
        m_recentRoomsGroup.sync();
    }

    onAccountSelectionChanged(ui->comboBox->currentIndex());
}

void KTp::JoinChatRoomDialog::addRecentRoom()
{
    Tp::AccountPtr account = ui->comboBox->currentAccount();
    if (!account || ui->lineEdit->text().isEmpty()) {
        return;
    }

    QString recentAccount = account->uniqueIdentifier();
    QString recentHandle = ui->lineEdit->text();
    const QString &key = recentHandle + recentAccount;

    QVariantList recent;
    recent.append(recentHandle);
    recent.append(recentAccount);

    if(m_favoriteRoomsGroup.keyList().contains(key) || m_recentRoomsGroup.keyList().contains(key)) {
        return;
    }

    m_recentRoomsGroup.writeEntry(key, recent);
    m_recentRoomsGroup.sync();
}


void KTp::JoinChatRoomDialog::clearRecentRooms()
{
    QString accountIdentifier = ui->comboBox->currentAccount()->uniqueIdentifier();

    KSharedConfigPtr commonConfig = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    commonConfig->deleteGroup(QLatin1String("RecentChatRooms"));
    commonConfig->sync();

    // Reload the model
    m_favoritesModel->clearRooms();
    loadFavoriteRooms();

    // Update the list
    onAccountSelectionChanged(ui->comboBox->currentIndex());
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
        ui->queryButton->setEnabled(true);
        ui->queryButton->setIcon(KIcon(QLatin1String("media-playback-start")));
        ui->queryButton->setText(i18nc("Button text", "Query"));
        ui->queryButton->setToolTip(i18nc("Tooltip text", "Start query"));
        connect(ui->queryButton, SIGNAL(clicked(bool)), this, SLOT(getRoomList()));
        disconnect(ui->queryButton, SIGNAL(clicked(bool)), this, SLOT(stopListing()));
    }
}

void KTp::JoinChatRoomDialog::onListing(bool isListing)
{
    if (isListing) {
        kDebug() << "listing";
        ui->queryButton->setEnabled(true);
        ui->queryButton->setIcon(KIcon(QLatin1String("media-playback-stop")));
        ui->queryButton->setText(i18nc("Button text", "Stop"));
        ui->queryButton->setToolTip(i18nc("Tooltip text", "Stop query"));
        disconnect(ui->queryButton, SIGNAL(clicked(bool)), this, SLOT(getRoomList()));
        connect(ui->queryButton, SIGNAL(clicked(bool)), this, SLOT(stopListing()));
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

void KTp::JoinChatRoomDialog::onFavoriteRoomSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (current.isValid()) {
        ui->lineEdit->setText(current.data(FavoriteRoomsModel::HandleNameRole).toString());
    }
}

void KTp::JoinChatRoomDialog::onRoomClicked(const QModelIndex &index)
{
    ui->lineEdit->setText(index.data(RoomsModel::HandleNameRole).toString());
}

QString KTp::JoinChatRoomDialog::selectedChatRoom() const
{
    return ui->lineEdit->text();
}

void KTp::JoinChatRoomDialog::onTextChanged(QString newText)
{
    button(Ok)->setEnabled(!newText.isEmpty());
}

void KTp::JoinChatRoomDialog::onStartChatFinished(Tp::PendingOperation *op)
{
    setJoinInProgress(false);
    if (op->isError()) {
        kDebug() << "failed to join room";
        kDebug() << op->errorName() << op->errorMessage();

        ui->feedbackWidget->setMessageType(KMessageWidget::KMessageWidget::Error);
        ui->feedbackWidget->setText(i18n("Could not join chatroom"));
        ui->feedbackWidget->animatedShow();
    } else {
        close();
    }
}

void KTp::JoinChatRoomDialog::setJoinInProgress(bool inProgress)
{
    m_joinInProgress = inProgress;
    mainWidget()->setEnabled(!inProgress);
    button(KDialog::Ok)->setEnabled(!inProgress);
    button(KDialog::Cancel)->setEnabled(!inProgress);
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
        // Keep compatibility with KTp 0.8 and previous
        if(favorite.size() == 3) {
            // Update the entry in the config file
            favorite.removeFirst();
            m_favoriteRoomsGroup.writeEntry(key, favorite);
            m_favoriteRoomsGroup.sync();
        }
        QString favoriteHandle = favorite.at(0).toString();
        QString favoriteAccount = favorite.at(1).toString();
        QVariantMap room;
        room.insert(QLatin1String("is-bookmarked"), true);
        room.insert(QLatin1String("handle-name"), favoriteHandle);
        room.insert(QLatin1String("account-identifier"), favoriteAccount);
        roomList.append(room);
    }

    Q_FOREACH (const QString &key, m_recentRoomsGroup.keyList()) {
        QVariantList recent = m_recentRoomsGroup.readEntry(key, QVariantList());
        if (recent.length() > 1) {
            QString recentHandle = recent.at(0).toString();
            QString recentAccount = recent.at(1).toString();
            QVariantMap room;
            room.insert(QLatin1String("is-bookmarked"), false);
            room.insert(QLatin1String("handle-name"), recentHandle);
            room.insert(QLatin1String("account-identifier"), recentAccount);
            roomList.append(room);
        } else {
            kDebug() << "Invalid recent rooms group:" << key;
        }
    }

    m_favoritesModel->addRooms(roomList);
}
