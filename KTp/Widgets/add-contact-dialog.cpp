/*
 * Add contact dialog
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2012 George Kiagiadakis <kiagiadakis.george@gmail.com>
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

#include <QObject>
#include <QCloseEvent>

#include <KMessageBox>
#include <KPushButton>
#include <KDebug>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/Filter>
#include <TelepathyQt/AccountSet>

namespace KTp {

class KTP_NO_EXPORT SubscribableAccountFilter : public Tp::AccountFilter
{
public:
    SubscribableAccountFilter() : Tp::AccountFilter()
    {
    }

    bool isValid() const
    {
        //return whether the filter is valid which is always true as this filter is hardcoded
        return true;
    }

    bool matches(const Tp::AccountPtr &account) const
    {
        //if there's no connection we can't add contacts as we have no contactmanager
        if (! account->connection()) {
            return false;
        }

        //only show items which can add a contact (i.e hide accounts like IRC which are online but there's no point listing)
        if (! account->connection()->contactManager()->canRequestPresenceSubscription()){
            return false;
        }
        return true;
    }
};


struct KTP_NO_EXPORT AddContactDialog::Private
{
    Private() :
        ui(new Ui::AddContactDialog),
        acceptInProgress(false)
    {}

    Ui::AddContactDialog *ui;
    bool acceptInProgress;
};

AddContactDialog::AddContactDialog(const Tp::AccountManagerPtr &accountManager, QWidget *parent) :
    KDialog(parent),
    d(new Private)
{
    setWindowTitle(i18n("Add new contact"));
    setWindowIcon(QIcon::fromTheme(QLatin1String("list-add-user")));

    QWidget *widget = new QWidget(this);
    d->ui->setupUi(widget);
    setMainWidget(widget);

    Tp::AccountFilterPtr filter = Tp::AccountFilterPtr(new KTp::SubscribableAccountFilter());
    Tp::AccountSetPtr accountSet = accountManager->filterAccounts(filter);

    d->ui->accountCombo->setAccountSet(accountSet);

    d->ui->screenNameLineEdit->setFocus();
}

AddContactDialog::~AddContactDialog()
{
    delete d->ui;
    delete d;
}

void AddContactDialog::accept()
{
    Tp::AccountPtr account = d->ui->accountCombo->currentAccount();

    if (account.isNull()) {
        KMessageBox::sorry(this, i18n("No account selected."));
    } else if (account->connection().isNull()) {
        KMessageBox::sorry(this, i18n("The requested account has been disconnected "
                                      "and so the contact could not be added."));
    } else if (d->ui->screenNameLineEdit->text().isEmpty()) {
        KMessageBox::sorry(this, i18n("You did not specify the name of the contact to add."));
    } else {
        QStringList identifiers = QStringList() << d->ui->screenNameLineEdit->text();
        kDebug() << "Requesting contacts for identifiers:" << identifiers;

        Tp::PendingContacts *pendingContacts = account->connection()->contactManager()->contactsForIdentifiers(identifiers);
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
        kWarning() << "Failed to retrieve a contact for the given identifier"
                   << op->errorName() << op->errorMessage();
        KMessageBox::sorry(this, i18n("Failed to create new contact."));
        setInProgress(false);
    } else {
        kDebug() << "Requesting presence subscription";

        Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);
        connect(pc->manager()->requestPresenceSubscription(pc->contacts()),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(_k_onRequestPresenceSubscriptionFinished(Tp::PendingOperation*)));
    }
}

void AddContactDialog::_k_onRequestPresenceSubscriptionFinished(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Failed to request presence subscription"
                   << op->errorName() << op->errorMessage();
        KMessageBox::sorry(this, i18n("Failed to request presence subscription "
                                      "from the requested contact."));
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
