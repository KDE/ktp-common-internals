/*
    Copyright (C) 2016  Martin Klapetek <mklapetek@kde.org>

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

#include "emojis-model.h"
#include "emojis.h"

#include <KEmoticonsTheme>
#include <KEmoticons>
#include <QDebug>

EmojisModel::EmojisModel(QObject *parent)
    : QAbstractListModel(parent)
{
    KEmoticons *kemoticons = new KEmoticons();
    m_theme = kemoticons->theme(QStringLiteral("EmojiOne"));
    m_themePath = m_theme.themePath();
    m_emojisHash = m_theme.emoticonsMap();
}

EmojisModel::~EmojisModel()
{
}

QHash<int, QByteArray> EmojisModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[EmojiFullPath] = "emojiFullPath";
    roles[EmojiImage] = "emojiImage";
    roles[EmojiText] = "emojiText";
    return roles;
}

QVariant EmojisModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const int row = index.row();
    const QString filePath = QStringLiteral("%1/%2.png").arg(m_themePath).arg(_s_emojisOrder.at(row));

    switch (role) {
        case EmojiFullPath:
            return filePath;
        case EmojiSection:
            if (row < 204) {
                return QStringLiteral("Smileys & People");
            } else if (row >= 204 && row < 352) {
                return QStringLiteral("Animals & Nature");
            } else if (row >= 352 && row < 419) {
                return QStringLiteral("Food & Drink");
            } else if (row >= 419 && row < 476) {
                return QStringLiteral("Activity");
            } else if (row >= 476 && row < 591) {
                return QStringLiteral("Travel & Places");
            } else if (row >= 591 && row < 769) {
                return QStringLiteral("Objects");
            } else if (row >= 769 && row < 1038) {
                return QStringLiteral("Symbols");
            } else if (row >= 1038 && row < 1295) {
                return QStringLiteral("Flags");
            } else if (row >= 1295) {
                return QStringLiteral("Diversity");
            }
        case EmojiText:
            const auto emojiValues = m_emojisHash.value(filePath);
            Q_FOREACH (const QString &emoji, emojiValues) {
                if (emoji.startsWith(QLatin1Char(':'))) {
                    return emoji;
                }
            }
            return emojiValues.at(0);
    }

    return QVariant();
}

int EmojisModel::rowCount(const QModelIndex &parent) const
{
    return _s_emojisOrder.size();
}
