/*
    Copyright 2014  Nilesh Suthar <nileshsuthar@live.in>

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

#ifndef CHATLISTVIEWDELEGATE_H
#define CHATLISTVIEWDELEGATE_H

#include <QStyledItemDelegate>

class QPainter;

class ChatListviewDelegate : public QStyledItemDelegate
{
public:
    ChatListviewDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

    enum dataRole {
        senderAliasRole = Qt::UserRole + 100,
        messageRole, messageTimeRole
    };

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

};
#endif // CHATLISTVIEWDELEGATE_H
