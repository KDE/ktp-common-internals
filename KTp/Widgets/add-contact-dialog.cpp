/*
 * Add contact dialog
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
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

#include "add-contact-dialog.h"
#include "ui_add-contact-dialog.h"

#include <KTp/Models/accounts-model.h>
#include <KTp/Models/accounts-model-item.h>

#include <QObject>
#include <QSortFilterProxyModel>
#include <QDebug>


#include <TelepathyQt/Account>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ContactManager>

namespace KTp {

/** A filter which only lists connections which accept adding contacts*/
class SubscribableAccountsModel : public QSortFilterProxyModel
{
public:
    SubscribableAccountsModel(QObject *parent);
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};

SubscribableAccountsModel::SubscribableAccountsModel(QObject *parent)
 : QSortFilterProxyModel(parent)
{
}

bool KTp::SubscribableAccountsModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    AccountsModelItem* item = sourceModel()->index(source_row, 0, source_parent).data(AccountsModel::ItemRole).value<AccountsModelItem*>();

    if (item) {
        Tp::AccountPtr account = item->account();

        //if there's no connection we can't add contacts as we have no contactmanager
        if (! account->connection()) {
            return false;
        }

        //only show items which can add a contact (i.e hide accounts like IRC which are online but there's no point listing)
        if (! account->connection()->contactManager()->canRequestPresenceSubscription()){
            return false;
        }
    }
    return true;
}

} //namespace

KTp::AddContactDialog::AddContactDialog(AccountsModel *accountModel, QWidget *parent) :
    KDialog(parent),
    ui(new Ui::AddContactDialog)
{
    QWidget *widget = new QWidget(this);
    ui->setupUi(widget);
    setMainWidget(widget);

    SubscribableAccountsModel *filteredModel = new SubscribableAccountsModel(this);
    filteredModel->setSourceModel(accountModel);
    for (int i = 0; i < filteredModel->rowCount(); ++i) {
        ui->accountCombo->addItem(KIcon(filteredModel->data(filteredModel->index(i, 0), AccountsModel::IconRole).toString()),
                                  filteredModel->data(filteredModel->index(i, 0)).toString(),
                                  filteredModel->data(filteredModel->index(i, 0), AccountsModel::ItemRole));
    }

    ui->screenNameLineEdit->setFocus();
}

KTp::AddContactDialog::~AddContactDialog()
{
    delete ui;
}

Tp::AccountPtr KTp::AddContactDialog::account() const
{
    QVariant itemData = ui->accountCombo->itemData(ui->accountCombo->currentIndex(),AccountsModel::ItemRole);
    AccountsModelItem* item = itemData.value<AccountsModelItem*>();
    if (item) {
        return item->account();
    } else {
        return Tp::AccountPtr();
    }
}

const QString KTp::AddContactDialog::screenName() const
{
    return ui->screenNameLineEdit->text();
}

#include "add-contact-dialog.moc"
