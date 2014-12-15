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

#include "rooms-model.h"
#include <QIcon>
#include <KLocalizedString>

// RoomsModel
KTp::RoomsModel::RoomsModel(QObject *parent): QAbstractListModel(parent)
{
}

int KTp::RoomsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_roomInfoList.size();
    }
}

int KTp::RoomsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return 4;
    }
}

QVariant KTp::RoomsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_roomInfoList.count()) {
        return QVariant();
    }

    const int row = index.row();
    const Tp::RoomInfo &roomInfo = m_roomInfoList.at(row);

    // this is handled here because when putting it in the switch below
    // all columns get an empty space for the decoration
    if (index.column() == PasswordColumn) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::DecorationRole:
            if (roomInfo.info.value(QLatin1String("password")).toBool()) {
                return QIcon::fromTheme(QStringLiteral("object-locked"));
            } else {
                return QVariant();
            }
        case Qt::ToolTipRole:
            if (roomInfo.info.value(QLatin1String("password")).toBool()) {
                return i18n("Password required");
            } else {
                return i18n("No password required");
            }
        }
    }

    switch(role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case NameColumn:
            return roomInfo.info.value(QLatin1String("name"));
        case DescriptionColumn:
            return roomInfo.info.value(QLatin1String("description"));
        case MembersColumn:
            return roomInfo.info.value(QLatin1String("members"));
        }
    case Qt::ToolTipRole:
        switch (index.column()) {
        case MembersColumn:
            return i18n("Member count");
        }
    case RoomsModel::HandleNameRole:
        return roomInfo.info.value(QLatin1String("handle-name"));
    }

    return QVariant();
}

QVariant KTp::RoomsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::DecorationRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        switch (role) {
        case Qt::DisplayRole:
            switch (section) {
            case NameColumn:
                return i18nc("Chatrooms name", "Name");
            case DescriptionColumn:
                return i18nc("Chatrooms description", "Description");
            }
        case Qt::DecorationRole:
            switch (section) {
            case PasswordColumn:
                return QIcon::fromTheme(QStringLiteral("object-locked"));
            case MembersColumn:
                return QIcon::fromTheme(QStringLiteral("meeting-participant"));
            }
        }
    }

    return QVariant();
}

void KTp::RoomsModel::addRooms(const Tp::RoomInfoList newRoomList)
{
    if (newRoomList.size() > 0) {
        beginInsertRows(QModelIndex(), m_roomInfoList.size(), m_roomInfoList.size() + newRoomList.size() - 1);
        m_roomInfoList.append(newRoomList);
        endInsertRows();
    }
}

void KTp::RoomsModel::clearRoomInfoList()
{
    if (m_roomInfoList.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_roomInfoList.size() - 1);
        m_roomInfoList.clear();
        endRemoveRows();
    }
}

// FavoriteRoomsModel
KTp::FavoriteRoomsModel::FavoriteRoomsModel(QObject *parent): QAbstractListModel(parent)
{
}

int KTp::FavoriteRoomsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_favoriteRoomsList.size();
    }
}

int KTp::FavoriteRoomsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return 3;
    }
}

QVariant KTp::FavoriteRoomsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_favoriteRoomsList.size()) {
        return QVariant();
    }

    const int row = index.row();
    const QVariantMap &room = m_favoriteRoomsList.at(row);

    switch(role) {
    case Qt::EditRole: // Return same values for both Display and Edit roles
    case Qt::DisplayRole:
        switch (index.column()) {
        case BookmarkColumn :
            return QVariant();
        case HandleNameColumn:
            return room.value(QLatin1String("handle-name"));
        case AccountIdentifierColumn:
            return room.value(QLatin1String("account-identifier"));
        }
    case Qt::ToolTipRole:
        switch (index.column()) {
        case BookmarkColumn:
            if (room.value(QLatin1String("is-bookmarked")).toBool()) {
                return i18n("Room bookmarked");
            } else {
                return i18n("Room not bookmarked");
            }
        case HandleNameColumn:
        case AccountIdentifierColumn:
            return room.value(QLatin1String("handle-name"));
        }
    case Qt::DecorationRole:
        switch (index.column()) {
        case BookmarkColumn:
            if (room.value(QLatin1String("is-bookmarked")).toBool()) {
                return QIcon::fromTheme(QStringLiteral("bookmarks"));
            } else {
                return QIcon(QIcon::fromTheme(QStringLiteral("bookmarks")).pixmap(32, 32, QIcon::Disabled));
            }
        case HandleNameColumn:
        case AccountIdentifierColumn:
            return QVariant();
        }
    case Qt::CheckStateRole:
        switch (index.column()) {
        case BookmarkColumn:
            return room.value(QLatin1String("is-bookmarked")).toBool() ? Qt::Checked : Qt::Unchecked;
        case HandleNameColumn:
        case AccountIdentifierColumn:
            return QVariant();
        }
    case FavoriteRoomsModel::BookmarkRole:
        room.value(QLatin1String("is-bookmarked")).toBool();
    case FavoriteRoomsModel::HandleNameRole:
        return room.value(QLatin1String("handle-name"));
    case FavoriteRoomsModel::AccountRole:
        return room.value(QLatin1String("account-identifier"));
    case FavoriteRoomsModel::FavoriteRoomRole:
        return QVariant::fromValue<QVariantMap>(room);
    }

    return QVariant();
}

bool KTp::FavoriteRoomsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_favoriteRoomsList.size()) {
        return false;
    }

    const int row = index.row();
    QVariantMap &room = m_favoriteRoomsList[row];

    if (role == Qt::EditRole) {
        switch (index.column()) {
        case BookmarkColumn:
            room.insert(QLatin1String("is-bookmarked"), value);
            break;
        case HandleNameColumn:
            room.insert(QLatin1String("handle-name"), value);
            break;
        case AccountIdentifierColumn:
            room.insert(QLatin1String("account-identifier"), value);
            break;
        default:
            return false;
        }
        Q_EMIT dataChanged(index, index);
        return true;
    }
    if (role == Qt::CheckStateRole) {
        switch (index.column()) {
        case BookmarkColumn:
            room.insert(QLatin1String("is-bookmarked"), value == Qt::Checked ? true : false);
            break;
        }
        Q_EMIT dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags KTp::FavoriteRoomsModel::flags(const QModelIndex &index) const {
    if (index.column() == BookmarkColumn) {
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void KTp::FavoriteRoomsModel::addRooms(const QList<QVariantMap> newRoomList)
{
    if (newRoomList.size() > 0) {
        beginInsertRows(QModelIndex(), m_favoriteRoomsList.size(), m_favoriteRoomsList.size() + newRoomList.size() - 1);
        m_favoriteRoomsList.append(newRoomList);
        endInsertRows();
    }
}

void KTp::FavoriteRoomsModel::addRoom(const QVariantMap &room)
{
    beginInsertRows(QModelIndex(), m_favoriteRoomsList.size(), m_favoriteRoomsList.size());
    m_favoriteRoomsList.append(room);
    endInsertRows();
}

void KTp::FavoriteRoomsModel::removeRoom(const QVariantMap &room)
{
    int row = m_favoriteRoomsList.indexOf(room);
    beginRemoveRows(QModelIndex(), row, row);
    m_favoriteRoomsList.removeOne(room);
    endRemoveRows();
}

void KTp::FavoriteRoomsModel::clearRooms()
{
    beginResetModel();
    m_favoriteRoomsList.clear();
    endResetModel();
}

bool KTp::FavoriteRoomsModel::containsRoom(const QString &handle, const QString &account) const
{
    bool contains = false;

    Q_FOREACH(const QVariantMap &room, m_favoriteRoomsList) {
        if ((room.value(QLatin1String("handle-name")) == handle) && (room.value(QLatin1String("account-identifier")) == account)) {
            contains = true;
        }
    }

    return contains;
}

int KTp::FavoriteRoomsModel::countForAccount(const QString &account) const
{
    int count = 0;

    Q_FOREACH (const QVariantMap &room, m_favoriteRoomsList) {
        if (room.value(QLatin1String("account-identifier")) == account) {
            count++;
        }
    }

    return count;
}
