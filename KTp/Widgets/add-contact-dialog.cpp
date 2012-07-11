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
#include <QCloseEvent>

#include <KMessageBox>
#include <KPushButton>
#include <KDebug>

#include <TelepathyQt/Account>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

namespace KTp {

/** A filter which only lists connections which accept adding contacts*/
class KTP_NO_EXPORT SubscribableAccountsModel : public QSortFilterProxyModel
{
public:
    SubscribableAccountsModel(QObject *parent);
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};

SubscribableAccountsModel::SubscribableAccountsModel(QObject *parent)
 : QSortFilterProxyModel(parent)
{
}

bool SubscribableAccountsModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
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


struct KTP_NO_EXPORT AddContactDialog::Private
{
    Private() :
        ui(new Ui::AddContactDialog),
        acceptInProgress(false)
    {}

    Ui::AddContactDialog *ui;
    bool acceptInProgress;
};

AddContactDialog::AddContactDialog(AccountsModel *accountModel, QWidget *parent) :
    KDialog(parent),
    d(new Private)
{
    QWidget *widget = new QWidget(this);
    d->ui->setupUi(widget);
    setMainWidget(widget);

    SubscribableAccountsModel *filteredModel = new SubscribableAccountsModel(this);
    filteredModel->setSourceModel(accountModel);
    for (int i = 0; i < filteredModel->rowCount(); ++i) {
        //TODO accountCombo->setModel()
        d->ui->accountCombo->addItem(KIcon(filteredModel->data(filteredModel->index(i, 0), AccountsModel::IconRole).toString()),
                                  filteredModel->data(filteredModel->index(i, 0)).toString(),
                                  filteredModel->data(filteredModel->index(i, 0), AccountsModel::ItemRole));
    }

    d->ui->screenNameLineEdit->setFocus();
}

AddContactDialog::~AddContactDialog()
{
    delete d->ui;
    delete d;
}

Tp::AccountPtr AddContactDialog::account() const
{
    QVariant itemData = d->ui->accountCombo->itemData(d->ui->accountCombo->currentIndex(),AccountsModel::ItemRole);
    AccountsModelItem* item = itemData.value<AccountsModelItem*>();
    if (item) {
        return item->account();
    } else {
        return Tp::AccountPtr();
    }
}

const QString AddContactDialog::screenName() const
{
    return d->ui->screenNameLineEdit->text();
}

void AddContactDialog::accept()
{
    Tp::AccountPtr acc = account();
    if (acc.isNull()) {
        KMessageBox::error(this,
                i18n("Seems like you forgot to select an account. Also do not forget to connect it first."),
                i18n("No Account Selected"));
    } else if (acc->connection().isNull()) {
        KMessageBox::error(this,
                i18n("The requested account has disconnected and so the contact could not be added. Sorry."),
                i18n("Connection Error"));
    } else {
        QStringList identifiers = QStringList() << d->ui->screenNameLineEdit->text();
        Tp::PendingContacts *pendingContacts = acc->connection()->contactManager()->contactsForIdentifiers(identifiers);
        connect(pendingContacts, SIGNAL(finished(Tp::PendingOperation*)),
                this, SLOT(_k_onContactsForIdentifiersFinished(Tp::PendingOperation*)));
        setInProgress(true);
    }
}

void AddContactDialog::closeEvent(QCloseEvent *e)
{
    // ignore close event if we are in the middle of an operation
    if (!d->acceptInProgress) {
        KDialog::closeEvent(e);
    }
}

void AddContactDialog::_k_onContactsForIdentifiersFinished(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kDebug() << "Failed to retrieve a contact for the given identifier"
                 << op->errorName() << op->errorMessage();
        KMessageBox::error(this, i18n("Failed to construct a contact with the given name"));
        setInProgress(false);
    } else {
        Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);
        connect(pc->manager()->requestPresenceSubscription(pc->contacts()),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(_k_onRequestPresenceSubscriptionFinished(Tp::PendingOperation*)));
    }
}

void AddContactDialog::_k_onRequestPresenceSubscriptionFinished(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kDebug() << "Failed to request presence subscription"
                 << op->errorName() << op->errorMessage();
        KMessageBox::error(this, i18n("Failed to request presence subscription from the requested contact"));
        setInProgress(false);
    } else {
        QDialog::accept();
    }
}

void AddContactDialog::setInProgress(bool inProgress)
{
    d->acceptInProgress = inProgress;
    mainWidget()->setEnabled(!inProgress);
    button(KDialog::Ok)->setEnabled(!inProgress);
    button(KDialog::Cancel)->setEnabled(!inProgress);
}

} //namespace KTp

#include "add-contact-dialog.moc"
