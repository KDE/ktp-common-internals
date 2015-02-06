/*
 * This file is part of telepathy-kde-models-test-ui
 *
 * Copyright (C) 2011 Collabora Ltd. <info@collabora.co.uk>
 * Copyright (C) 2013 David Edmundson <davidedmundson@kde.org>
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

#include "model-view.h"

#include "roles-proxy-model.h"
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QFontDatabase>
#include <QDebug>

#include <KTp/types.h>

class SimpleDelegate : public QStyledItemDelegate {
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

QSize SimpleDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(24, 24);
}

void SimpleDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, index);
    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &optV4, painter);

    QRect iconRect = optV4.rect;
    iconRect.setSize(QSize(22, 22));
    iconRect.moveTo(QPoint(iconRect.x() + 8, iconRect.y() + 8));

    QPixmap avatar(qvariant_cast<QPixmap>(index.data(KTp::ContactAvatarPixmapRole)));

    if (!avatar.isNull()) {
        style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, avatar.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    QPixmap icon = QIcon::fromTheme(index.data(KTp::ContactPresenceIconRole).toString()).pixmap(22);

    QRect statusIconRect = optV4.rect;
    statusIconRect.setSize(QSize(22, 22));
    statusIconRect.moveTo(QPoint(optV4.rect.right() - 24,
                                 optV4.rect.top() + (optV4.rect.height() - 22) / 2));

    painter->drawPixmap(statusIconRect, icon);

    QFont nameFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    const QFontMetrics nameFontMetrics(nameFont);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    }

    painter->setFont(nameFont);

    QRect userNameRect = optV4.rect;
    userNameRect.setX(iconRect.x() + iconRect.width() + 18);
    userNameRect.setY(userNameRect.y() + (userNameRect.height()/2 - nameFontMetrics.height()/2));
    userNameRect.setWidth(userNameRect.width() - 22);

    QString nameText = index.data(Qt::DisplayRole).toString();

    painter->drawText(userNameRect,
                      nameFontMetrics.elidedText(nameText, Qt::ElideRight, userNameRect.width()));

    painter->restore();
}


ModelView::ModelView(QAbstractItemModel *model, QWidget *parent)
  : QWidget(parent)
{
    setupUi(this);

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setDynamicSortFilter(true);
    proxy->setSourceModel(model);
    proxy->setSortRole(Qt::DisplayRole);
    proxy->sort(0);

    TreeView->setModel(proxy);
    TreeView->setUniformRowHeights(true);
    TreeView->setItemDelegate(new SimpleDelegate());

    connect(TreeView, &QAbstractItemView::clicked, [this] (const QModelIndex &index) {
        bool isPerson = index.data(KTp::RowTypeRole).toUInt() == KTp::PersonRowType;
        QString personString = isPerson ? QStringLiteral("Yes, ") + QString::number(index.model()->rowCount(index)) + QStringLiteral(" subcontacts") : QStringLiteral("No");

        qDebug() << "Contact info";
        qDebug() << "------------";
        qDebug() << "        ID:" << index.data(KTp::IdRole).toString();
        qDebug() << " Person ID:" << index.data(KTp::PersonIdRole).toString();
        qDebug() << "  Username:" << index.data(Qt::DisplayRole).toString();
        qDebug() << "   Blocked:" << index.data(KTp::ContactIsBlockedRole).toString();
        qDebug() << "    Groups:" << index.data(KTp::ContactGroupsRole).toStringList();
        qDebug() << " Is Person:" << personString;
        qDebug();
    });
}

ModelView::~ModelView()
{

}

