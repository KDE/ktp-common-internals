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

#ifndef EMOJIS_MODEL_H
#define EMOJIS_MODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QHash>
#include <QStringList>

#include <KEmoticonsTheme>

class EmojisModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        EmojiFullPath = Qt::UserRole,
        EmojiImage,
        EmojiSection,
        EmojiText
    };
    Q_ENUMS(Role)

    EmojisModel(QObject *parent = 0);
    virtual ~EmojisModel();

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private:
    KEmoticonsTheme m_theme;
    QString m_themePath;
    QHash<QString, QStringList> m_emojisHash;
};

#endif // EMOJIS_MODEL_H
