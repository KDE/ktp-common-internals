/*
 * Rooms Model - A model of chatrooms.
 * Copyright (C) 2012  Dominik Cermak <d.cermak@arcor.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef ROOMS_MODEL_H
#define ROOMS_MODEL_H

#include <QtCore/QAbstractListModel>
#include <TelepathyQt/Types>

#include <KTp/Models/ktpmodels_export.h>

namespace KTp {

class KTPMODELS_EXPORT RoomsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // TODO: find a suitable icon and add an invitation column
    enum Column {
        NameColumn=0,
        DescriptionColumn,
        MembersColumn,
        PasswordColumn,
    };

    enum Roles {
        HandleNameRole = Qt::UserRole
    };

    explicit RoomsModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * \brief Add new rooms to the list.
     *
     * \param newRoomList The list with the new rooms to add.
     */
    void addRooms(const Tp::RoomInfoList newRoomList);

    /**
     * \brief Clear the room list.
     */
    void clearRoomInfoList();

private:
    Tp::RoomInfoList m_roomInfoList;
};

class KTPMODELS_EXPORT FavoriteRoomsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Column {
        BookmarkColumn = 0,
        HandleNameColumn,
        AccountIdentifierColumn
    };

    enum Roles {
        HandleNameRole = Qt::UserRole,
        BookmarkRole,
        AccountRole,
        FavoriteRoomRole
    };

    explicit FavoriteRoomsModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * \brief Add new rooms to the list.
     *
     * \param newRoomList The list with the new rooms to add.
     */
    void addRooms(const QList<QVariantMap> newRoomList);

    /**
     * \brief Add a new room to the list.
     *
     * \param room The room to add.
     */
    void addRoom(const QVariantMap &room);

    /**
     * \brief Remove a room from the list.
     *
     * \param room The room to remove.
     */
    void removeRoom(const QVariantMap &room);

    /**
     * \brief Remove all rooms from the list.
     */
    void clearRooms();

    /**
     * \brief Checks if it contains a room (identified by his handle-name).
     *
     * \param handle The handle to look for.
     *
     * \return True if it contains the room else false.
     */
    bool containsRoom(const QString &handle, const QString &account) const;

    /**
     * \brief Returns the count of rooms for the specified account.
     *
     * \param account The account to return the count for.
     *
     * \return The count of rooms.
     */
    int countForAccount(const QString &account) const;

private:
    QList<QVariantMap> m_favoriteRoomsList;
};

} // namespace KTp

#endif // ROOMS_MODEL_H
