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
#include "debug.h"

#include <KTp/Models/rooms-model.h>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KMessageWidget>
#include <KLocalizedString>
#include <KNotification>
#include <KIconLoader>

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
#include <QDialogButtonBox>

class KTp::JoinChatRoomDialog::Private
{
public:
    Private(JoinChatRoomDialog *q) :
        ui(new Ui::JoinChatRoomDialog)
        , model(new RoomsModel(q))
        , favoritesModel(new FavoriteRoomsModel(q))
        , favoritesProxyModel(new QSortFilterProxyModel(q))
        , joinInProgress(false)
    {}

    QList<Tp::AccountPtr> accounts;
    Ui::JoinChatRoomDialog *ui;
    QDialogButtonBox *buttonBox;
    Tp::PendingChannel *pendingRoomListChannel;
    Tp::ChannelPtr roomListChannel;
    Tp::Client::ChannelTypeRoomListInterface *iface;
    RoomsModel *model;
    FavoriteRoomsModel *favoritesModel;
    QSortFilterProxyModel *favoritesProxyModel;
    KConfigGroup favoriteRoomsGroup;
    KConfigGroup recentRoomsGroup;
    bool joinInProgress;
};

KTp::JoinChatRoomDialog::JoinChatRoomDialog(Tp::AccountManagerPtr accountManager, QWidget* parent)
    : QDialog(parent, Qt::Dialog)
    , d(new Private(this))
{
    QWidget *joinChatRoomDialog = new QWidget(this);
    d->ui->setupUi(joinChatRoomDialog);
    d->ui->feedbackWidget->hide();

    d->buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(joinChatRoomDialog);
    mainLayout->addWidget(d->buttonBox);
    setLayout(mainLayout);

    setWindowIcon(QIcon::fromTheme(QLatin1String("im-irc")));
    setWindowTitle(i18nc("Dialog title", "Join Chat Room"));

    d->ui->filterPicture->clear();
    d->ui->filterPicture->setPixmap(KIconLoader::global()->loadIcon(QLatin1String("view-filter"), KIconLoader::Small));

    // config
    KSharedConfigPtr commonConfig = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    d->favoriteRoomsGroup = commonConfig->group(QLatin1String("FavoriteRooms"));
    d->recentRoomsGroup = commonConfig->group(QLatin1String("RecentChatRooms"));

    // load favorite and recent rooms
    loadFavoriteRooms();

    // disable OK button on start
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    d->buttonBox->button(QDialogButtonBox::Ok)->setText(i18nc("button", "Join/Create"));
    d->buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme(QLatin1String("im-irc")));

    onAccountSelectionChanged(d->ui->comboBox->currentIndex());
    connect(accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    // Previous Tab
    d->favoritesProxyModel->setSourceModel(d->favoritesModel);
    d->favoritesProxyModel->setFilterKeyColumn(FavoriteRoomsModel::AccountIdentifierColumn);
    d->favoritesProxyModel->setSortRole(Qt::CheckStateRole);
    d->favoritesProxyModel->setDynamicSortFilter(true);

    d->ui->previousView->setModel(d->favoritesProxyModel);
    d->ui->previousView->setHeaderHidden(true);
    d->ui->previousView->header()->setStretchLastSection(false);
    d->ui->previousView->header()->setSectionResizeMode(FavoriteRoomsModel::BookmarkColumn, QHeaderView::ResizeToContents);
    d->ui->previousView->header()->setSectionResizeMode(FavoriteRoomsModel::HandleNameColumn, QHeaderView::Stretch);
    d->ui->previousView->setColumnHidden(FavoriteRoomsModel::AccountIdentifierColumn, true);
    d->ui->previousView->sortByColumn(FavoriteRoomsModel::BookmarkColumn, Qt::DescendingOrder);

    // Search Tab
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(d->model);
    proxyModel->setSortLocaleAware(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(RoomsModel::NameColumn);
    proxyModel->setDynamicSortFilter(true);

    d->ui->queryView->setModel(proxyModel);
    d->ui->queryView->header()->setStretchLastSection(false);
    d->ui->queryView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    d->ui->queryView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    d->ui->queryView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    d->ui->queryView->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    d->ui->queryView->header()->setSortIndicatorShown(false);
    d->ui->queryView->sortByColumn(RoomsModel::NameColumn, Qt::AscendingOrder);

    // connects
    connect(d->ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));
    connect(d->ui->previousView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
    connect(d->ui->previousView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(onFavoriteRoomSelectionChanged(QModelIndex,QModelIndex)));
    connect(d->favoritesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(onFavoriteRoomDataChanged(QModelIndex,QModelIndex)));
    connect(d->ui->clearRecentPushButton, SIGNAL(clicked(bool)), this, SLOT(clearRecentRooms()));
    connect(d->ui->serverLineEdit, SIGNAL(returnPressed()), this, SLOT(getRoomList()));
    connect(d->ui->queryButton, SIGNAL(clicked(bool)), this, SLOT(getRoomList()));
    connect(d->ui->queryView, SIGNAL(clicked(QModelIndex)), this, SLOT(onRoomClicked(QModelIndex)));
    connect(d->ui->queryView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
    connect(d->ui->filterBar, SIGNAL(textChanged(QString)), proxyModel, SLOT(setFilterFixedString(QString)));
    connect(d->ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAccountSelectionChanged(int)));
    connect(d->buttonBox, SIGNAL(accepted()), this, SLOT(addRecentRoom()));
    connect(d->buttonBox, SIGNAL(accepted()), this, SLOT(accept())); //FIXME?
    connect(d->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void KTp::JoinChatRoomDialog::closeEvent(QCloseEvent* e)
{
    // ignore close event if we are in the middle of an operation
    if (!d->joinInProgress) {
        QDialog::closeEvent(e);
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

    d->ui->comboBox->setAccountSet(accountManager->filterAccounts(filter));

    // queryTab
    if (d->ui->comboBox->count() > 0) {
        d->ui->queryButton->setEnabled(true);
    }

    // apply the filter after populating
    onAccountSelectionChanged(d->ui->comboBox->currentIndex());
}

KTp::JoinChatRoomDialog::~JoinChatRoomDialog()
{
    delete d->ui;
    delete d;
}

Tp::AccountPtr KTp::JoinChatRoomDialog::selectedAccount() const
{
    return d->ui->comboBox->currentAccount();
}

void KTp::JoinChatRoomDialog::accept()
{
    d->ui->feedbackWidget->hide();
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

    if (!d->ui->comboBox->currentAccount()) {
        // Set a filter expression that matches no account identifier
        d->favoritesProxyModel->setFilterRegExp(QLatin1String("a^"));
        return;
    }

    QString accountIdentifier = d->ui->comboBox->currentAccount()->uniqueIdentifier();
    d->favoritesProxyModel->setFilterFixedString(accountIdentifier);

    // Enable/disable the buttons as appropriate
    d->ui->clearRecentPushButton->setEnabled(!d->recentRoomsGroup.keyList().empty());
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
        if (d->recentRoomsGroup.keyList().contains(key)) {
            d->recentRoomsGroup.deleteEntry(key);
            d->recentRoomsGroup.sync();
        }
        d->favoriteRoomsGroup.writeEntry(key, favorite);
        d->favoriteRoomsGroup.sync();
    } else {
        if (d->favoriteRoomsGroup.keyList().contains(key)) {
            d->favoriteRoomsGroup.deleteEntry(key);
            d->favoriteRoomsGroup.sync();
        }
        d->recentRoomsGroup.writeEntry(key, favorite);
        d->recentRoomsGroup.sync();
    }

    onAccountSelectionChanged(d->ui->comboBox->currentIndex());
}

void KTp::JoinChatRoomDialog::addRecentRoom()
{
    Tp::AccountPtr account = d->ui->comboBox->currentAccount();
    if (!account || d->ui->lineEdit->text().isEmpty()) {
        return;
    }

    QString recentAccount = account->uniqueIdentifier();
    QString recentHandle = d->ui->lineEdit->text();
    const QString &key = recentHandle + recentAccount;

    QVariantList recent;
    recent.append(recentHandle);
    recent.append(recentAccount);

    if(d->favoriteRoomsGroup.keyList().contains(key) || d->recentRoomsGroup.keyList().contains(key)) {
        return;
    }

    d->recentRoomsGroup.writeEntry(key, recent);
    d->recentRoomsGroup.sync();
}


void KTp::JoinChatRoomDialog::clearRecentRooms()
{
    QString accountIdentifier = d->ui->comboBox->currentAccount()->uniqueIdentifier();

    KSharedConfigPtr commonConfig = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    commonConfig->deleteGroup(QLatin1String("RecentChatRooms"));
    commonConfig->sync();

    // Reload the model
    d->favoritesModel->clearRooms();
    loadFavoriteRooms();

    // Update the list
    onAccountSelectionChanged(d->ui->comboBox->currentIndex());
}

void KTp::JoinChatRoomDialog::getRoomList()
{
    Tp::AccountPtr account = d->ui->comboBox->currentAccount();
    if (!account) {
        return;
    }

    // Clear the list from previous items
    d->model->clearRoomInfoList();

    // Build the channelrequest
    QVariantMap request;
    request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".ChannelType"),
                   TP_QT_IFACE_CHANNEL_TYPE_ROOM_LIST);
    request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType"),
                   Tp::HandleTypeNone);

    // If the user provided a server use it, else use the standard server for the selected account
    if (!d->ui->serverLineEdit->text().isEmpty()) {
        request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".Type.RoomList.Server"),
                       d->ui->serverLineEdit->text());
    }

    d->pendingRoomListChannel = account->createAndHandleChannel(request, QDateTime::currentDateTime());
    connect(d->pendingRoomListChannel, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onRoomListChannelReadyForHandling(Tp::PendingOperation*)));

}

void KTp::JoinChatRoomDialog::stopListing()
{
    d->iface->StopListing();
}

void KTp::JoinChatRoomDialog::onRoomListChannelReadyForHandling(Tp::PendingOperation *operation)
{
    if (operation->isError()) {
        qCDebug(KTP_WIDGETS) << operation->errorName();
        qCDebug(KTP_WIDGETS) << operation->errorMessage();
        QString errorMsg(operation->errorName() + QLatin1String(": ") + operation->errorMessage());
        sendNotificationToUser(errorMsg);
    } else {
        d->roomListChannel = d->pendingRoomListChannel->channel();

        connect(d->roomListChannel->becomeReady(),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onRoomListChannelReady(Tp::PendingOperation*)));
    }
}

void KTp::JoinChatRoomDialog::onRoomListChannelReady(Tp::PendingOperation *operation)
{
    if (operation->isError()) {
        qCDebug(KTP_WIDGETS) << operation->errorName();
        qCDebug(KTP_WIDGETS) << operation->errorMessage();
        QString errorMsg(operation->errorName() + QLatin1String(": ") + operation->errorMessage());
        sendNotificationToUser(errorMsg);
    } else {
        d->iface = d->roomListChannel->interface<Tp::Client::ChannelTypeRoomListInterface>();

        d->iface->ListRooms();

        connect(d->iface, SIGNAL(ListingRooms(bool)), SLOT(onListing(bool)));
        connect(d->iface, SIGNAL(GotRooms(Tp::RoomInfoList)), SLOT(onGotRooms(Tp::RoomInfoList)));
    }
}

void KTp::JoinChatRoomDialog::onRoomListChannelClosed(Tp::PendingOperation *operation)
{
    if (operation->isError()) {
        qCDebug(KTP_WIDGETS) << operation->errorName();
        qCDebug(KTP_WIDGETS) << operation->errorMessage();
        QString errorMsg(operation->errorName() + QLatin1String(": ") + operation->errorMessage());
        sendNotificationToUser(errorMsg);
    } else {
        d->ui->queryButton->setEnabled(true);
        d->ui->queryButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
        d->ui->queryButton->setText(i18nc("Button text", "Query"));
        d->ui->queryButton->setToolTip(i18nc("Tooltip text", "Start query"));
        connect(d->ui->queryButton, SIGNAL(clicked(bool)), this, SLOT(getRoomList()));
        disconnect(d->ui->queryButton, SIGNAL(clicked(bool)), this, SLOT(stopListing()));
    }
}

void KTp::JoinChatRoomDialog::onListing(bool isListing)
{
    if (isListing) {
        qCDebug(KTP_WIDGETS) << "listing";
        d->ui->queryButton->setEnabled(true);
        d->ui->queryButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-stop")));
        d->ui->queryButton->setText(i18nc("Button text", "Stop"));
        d->ui->queryButton->setToolTip(i18nc("Tooltip text", "Stop query"));
        disconnect(d->ui->queryButton, SIGNAL(clicked(bool)), this, SLOT(getRoomList()));
        connect(d->ui->queryButton, SIGNAL(clicked(bool)), this, SLOT(stopListing()));
    } else {
        qCDebug(KTP_WIDGETS) << "finished listing";
        Tp::PendingOperation *op =  d->roomListChannel->requestClose();
        connect(op,
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onRoomListChannelClosed(Tp::PendingOperation*)));
    }
}

void KTp::JoinChatRoomDialog::onGotRooms(Tp::RoomInfoList roomInfoList)
{
    d->model->addRooms(roomInfoList);
}

void KTp::JoinChatRoomDialog::onFavoriteRoomSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (current.isValid()) {
        d->ui->lineEdit->setText(current.data(FavoriteRoomsModel::HandleNameRole).toString());
    }
}

void KTp::JoinChatRoomDialog::onRoomClicked(const QModelIndex &index)
{
    d->ui->lineEdit->setText(index.data(RoomsModel::HandleNameRole).toString());
}

QString KTp::JoinChatRoomDialog::selectedChatRoom() const
{
    return d->ui->lineEdit->text();
}

void KTp::JoinChatRoomDialog::onTextChanged(QString newText)
{
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!newText.isEmpty());
}

void KTp::JoinChatRoomDialog::onStartChatFinished(Tp::PendingOperation *op)
{
    setJoinInProgress(false);
    if (op->isError()) {
        qCDebug(KTP_WIDGETS) << "failed to join room";
        qCDebug(KTP_WIDGETS) << op->errorName() << op->errorMessage();

        d->ui->feedbackWidget->setMessageType(KMessageWidget::KMessageWidget::Error);
        d->ui->feedbackWidget->setText(i18n("Could not join chatroom"));
        d->ui->feedbackWidget->animatedShow();
    } else {
        close();
    }
}

void KTp::JoinChatRoomDialog::setJoinInProgress(bool inProgress)
{
    d->joinInProgress = inProgress;
    layout()->parentWidget()->setEnabled(!inProgress);
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!inProgress);
    d->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(!inProgress);
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

    Q_FOREACH(const QString &key, d->favoriteRoomsGroup.keyList()) {
        QVariantList favorite = d->favoriteRoomsGroup.readEntry(key, QVariantList());
        // Keep compatibility with KTp 0.8 and previous
        if(favorite.size() == 3) {
            // Update the entry in the config file
            favorite.removeFirst();
            d->favoriteRoomsGroup.writeEntry(key, favorite);
            d->favoriteRoomsGroup.sync();
        }
        QString favoriteHandle = favorite.at(0).toString();
        QString favoriteAccount = favorite.at(1).toString();
        QVariantMap room;
        room.insert(QLatin1String("is-bookmarked"), true);
        room.insert(QLatin1String("handle-name"), favoriteHandle);
        room.insert(QLatin1String("account-identifier"), favoriteAccount);
        roomList.append(room);
    }

    Q_FOREACH (const QString &key, d->recentRoomsGroup.keyList()) {
        QVariantList recent = d->recentRoomsGroup.readEntry(key, QVariantList());
        QString recentHandle = recent.at(0).toString();
        QString recentAccount = recent.at(1).toString();
        QVariantMap room;
        room.insert(QLatin1String("is-bookmarked"), false);
        room.insert(QLatin1String("handle-name"), recentHandle);
        room.insert(QLatin1String("account-identifier"), recentAccount);
        roomList.append(room);
    }

    d->favoritesModel->addRooms(roomList);
}
