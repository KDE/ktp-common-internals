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



ModelView::ModelView(QAbstractItemModel *model, QWidget *parent)
  : QWidget(parent)
{
    setupUi(this);

//     RolesProxyModel *proxyModel = new RolesProxyModel(this);
//     proxyModel->setSourceModel(model);

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setDynamicSortFilter(true);
    proxy->setSourceModel(model);
    proxy->setSortRole(Qt::DisplayRole);

    TreeView->setModel(proxy);
}

ModelView::~ModelView()
{

}

