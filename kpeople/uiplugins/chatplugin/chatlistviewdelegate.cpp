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

#include "chatlistviewdelegate.h"
#include <QPainter>
#include <QApplication>

void ChatListviewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    painter->save();

    QFont font = QApplication::font();
    QFont timeFont = QApplication::font();
    QFont messagefont = QApplication::font();
    timeFont.setItalic(true);
    font.setBold(true);
    QFontMetrics fm(font);

    QString senderAlias = index.data(senderAliasRole).toString() + QLatin1String(":");
    QString message = index.data(messageRole).toString();
    QString time = index.data(messageTimeRole).toString();

    QRect aliasRect = option.rect;
    QRect messageRect = option.rect;
    QRect timeRect = option.rect;

    messageRect.setLeft(fm.width(senderAlias));

    painter->setFont(font);
    painter->drawText(aliasRect, senderAlias);

    painter->setFont(messagefont);
    painter->drawText(messageRect, message);

    painter->setFont(timeFont);
    painter->drawText(timeRect, Qt::AlignRight, time);

    painter->restore();

}

#include "chatlistviewdelegate.moc"
