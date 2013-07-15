/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2011 Francesco Nwokeka <francesco.nwokeka@gmail.com>
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

#ifndef JOINCHATROOMDIALOG_H
#define JOINCHATROOMDIALOG_H

#include <TelepathyQt/AccountManager>
#include <KDialog>
#include <KTp/Models/rooms-model.h>

#include <KTp/ktp-export.h>

namespace Ui {
    class JoinChatRoomDialog;
}

class RoomsModel;
class FavoriteRoomsModel;
class QSortFilterProxyModel;
class KCompletion;

namespace KTp {

class KTP_EXPORT JoinChatRoomDialog : public KDialog
{
    Q_OBJECT

public:
    explicit JoinChatRoomDialog(Tp::AccountManagerPtr accountManager, QWidget *parent = 0);
    ~JoinChatRoomDialog();

    Tp::AccountPtr selectedAccount() const;     /** returns selected account */
    QString selectedChatRoom() const;           /** returns selected chat room */

private Q_SLOTS:
    void onTextChanged(QString newText);
    void onAccountSelectionChanged(int newIndex);
    void addFavorite();
    void removeFavorite();
    void addRecentRoom();
    void removeRecentRoom();
    void clearRecentRooms();
    void getRoomList();
    void stopListing();
    void onRoomListChannelReadyForHandling(Tp::PendingOperation *operation);
    void onRoomListChannelReady(Tp::PendingOperation *operation);
    void onRoomListChannelClosed(Tp::PendingOperation *operation);
    void onListing(bool isListing);
    void onGotRooms(Tp::RoomInfoList roomInfoList);
    void onFavoriteRoomClicked(const QModelIndex &index);
    void onRecentRoomClicked();
    void onRoomClicked(const QModelIndex &index);
    void onAccountManagerReady(Tp::PendingOperation*);

private:
    void sendNotificationToUser(const QString& errorMsg);
    void loadFavoriteRooms();

    QList<Tp::AccountPtr> m_accounts;
    Ui::JoinChatRoomDialog *ui;
    Tp::PendingChannel *m_pendingRoomListChannel;
    Tp::ChannelPtr m_roomListChannel;
    Tp::Client::ChannelTypeRoomListInterface *m_iface;
    RoomsModel *m_model;
    FavoriteRoomsModel *m_favoritesModel;
    QSortFilterProxyModel *m_favoritesProxyModel;
    KConfigGroup m_favoriteRoomsGroup;
    KConfigGroup m_recentRoomsGroup;
    QHash <QString, QStringList> m_recentRooms;
    KCompletion *m_recentComp;

};

} //namespace KTp

#endif  // JOINCHATROOMDIALOG_H
