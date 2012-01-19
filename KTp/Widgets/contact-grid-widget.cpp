/*
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2012 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
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


#include "contact-grid-widget.h"

#include <KDE/KIcon>
#include <KDE/KLineEdit>
#include <KDE/KDebug>

#include <QtGui/QApplication>
#include <QtGui/QTextOption>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QListView>

#include <KTp/Models/accounts-model.h>
#include <KTp/Models/accounts-filter-model.h>
#include <KTp/Models/contact-model-item.h>
#include <KTp/Models/flat-model-proxy.h>


class KTp::ContactGridDelegate::Private
{
public:
    Private(KTp::ContactGridDelegate *parent)
        : q(parent)
    {
    }

    ~Private()
    {
    }

    KTp::ContactGridDelegate *q;
};

KTp::ContactGridDelegate::ContactGridDelegate(QObject *parent)
    : QAbstractItemDelegate(parent),
      d(new KTp::ContactGridDelegate::Private(this))
{
}

KTp::ContactGridDelegate::~ContactGridDelegate()
{
    delete d;
}

void KTp::ContactGridDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyle *style = QApplication::style();
    int textHeight = option.fontMetrics.height() * 2;

    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    QRect avatarRect = option.rect.adjusted(0, 0, 0, -textHeight);
    QRect textRect = option.rect.adjusted(0, option.rect.height() - textHeight, 0, -3);

    QPixmap avatar = index.data(Qt::DecorationRole).value<QPixmap>();
    if (avatar.isNull()) {
        avatar = KIcon(QLatin1String("im-user-online")).pixmap(QSize(70, 70));
    }

    //resize larger avatars
    if (avatar.width() > 80 || avatar.height() > 80) {
        avatar = avatar.scaled(QSize(80, 80), Qt::KeepAspectRatio);
        //draw leaving paddings on smaller (or non square) avatars
    }
    style->drawItemPixmap(painter, avatarRect, Qt::AlignCenter, avatar);


    QTextOption textOption;
    textOption.setAlignment(Qt::AlignCenter);
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    painter->drawText(textRect, index.data().toString(), textOption);

}

QSize KTp::ContactGridDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    int textHeight = option.fontMetrics.height() * 2;
    return QSize(84, 80 + textHeight + 3);
}

// -----------------------------------------------------------------------------

class KTp::ContactGridWidget::Private
{
public:
    Private(KTp::ContactGridWidget *parent)
        : q(parent),
          layout(new QVBoxLayout(parent)),
          contactGridView(new QListView(parent)),
          contactFilterLineEdit(new KLineEdit(parent)),
          accountsModel(0),
          filterModel(0),
          flatProxyModel(0)
    {
    }

    ~Private()
    {
    }

    void _k_onSelectionChanged(QItemSelection,QItemSelection);

    KTp::ContactGridWidget *q;
    QVBoxLayout *layout;
    QListView *contactGridView;
    KLineEdit *contactFilterLineEdit;
    AccountsModel *accountsModel;
    AccountsFilterModel *filterModel;
    FlatModelProxy *flatProxyModel;
};

void KTp::ContactGridWidget::Private::_k_onSelectionChanged(QItemSelection newSelection, QItemSelection oldSelection)
{
    Q_UNUSED(oldSelection)
    kDebug() << newSelection << oldSelection;

    if (newSelection.isEmpty()) {
        Q_EMIT q->selectionChanged(Tp::AccountPtr(), Tp::ContactPtr());
        return;
    }

    kDebug() << "Emitting selectionChanged" << q->selectedAccount()->displayName() << q->selectedContact()->alias();
    Q_EMIT q->selectionChanged(q->selectedAccount(), q->selectedContact());
}

// -----------------------------------------------------------------------------

KTp::ContactGridWidget::ContactGridWidget(AccountsModel* model, QWidget *parent)
    : QWidget(parent),
      d(new Private(this))
{
    d->filterModel = new AccountsFilterModel(this);
    d->flatProxyModel = new FlatModelProxy(d->filterModel);

    d->accountsModel = model;
    d->filterModel->setSourceModel(d->accountsModel);

    d->contactGridView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->contactGridView->setResizeMode(QListView::Adjust);
    d->contactGridView->setSpacing(5);
    d->contactGridView->setViewMode(QListView::IconMode);

    d->contactFilterLineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    d->contactFilterLineEdit->setClearButtonShown(true);

    d->layout->setMargin(0);
    d->layout->addWidget(d->contactGridView);
    d->layout->addWidget(d->contactFilterLineEdit);
    setLayout(d->layout);

    d->contactGridView->setModel(d->flatProxyModel);
    d->contactGridView->setItemDelegate(new ContactGridDelegate(this));

    connect(d->contactGridView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(_k_onSelectionChanged(QItemSelection,QItemSelection)));

    connect(d->contactFilterLineEdit,
            SIGNAL(textChanged(QString)),
            d->filterModel,
            SLOT(setFilterString(QString)));
}

KTp::ContactGridWidget::~ContactGridWidget()
{
    delete d;
}

bool KTp::ContactGridWidget::hasSelection() const
{
    return d->contactGridView->selectionModel()->hasSelection();
}

Tp::AccountPtr KTp::ContactGridWidget::selectedAccount() const
{
    return d->accountsModel->accountForContactItem(d->contactGridView->currentIndex().data(AccountsModel::ItemRole).value<ContactModelItem*>());
}

Tp::ContactPtr KTp::ContactGridWidget::selectedContact() const
{
    return d->contactGridView->currentIndex().data(AccountsModel::ItemRole).value<ContactModelItem*>()->contact();
}

void KTp::ContactGridWidget::setShowOfflineUsers(bool showOfflineUsers)
{
    d->filterModel->setShowOfflineUsers(showOfflineUsers);
}

void KTp::ContactGridWidget::setFilterByFileTransferCapability(bool filterByFileTransferCapability)
{
    d->filterModel->setFilterByFileTransferCapability(filterByFileTransferCapability);
}

void KTp::ContactGridWidget::setNameFilterBarShown(bool nameFilterBarShown)
{
    d->contactFilterLineEdit->clear();
    d->contactFilterLineEdit->setShown(nameFilterBarShown);
}

void KTp::ContactGridWidget::showNameFilterBar()
{
    setNameFilterBarShown(true);
}

void KTp::ContactGridWidget::hideNameFilterBar()
{
    setNameFilterBarShown(false);
}

void KTp::ContactGridWidget::setNameFilter(const QString& nameFilter)
{
    d->contactFilterLineEdit->setText(nameFilter);
}


#include "contact-grid-widget.moc"
