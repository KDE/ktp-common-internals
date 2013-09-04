/*
 * Start chat dialog
 *
 * Copyright (C) 2013 Anant Kamath <kamathanant@gmail.com>
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

#include "start-chat-dialog.h"
#include "ui_start-chat-dialog.h"

#include <QObject>
#include <QCloseEvent>

#include <KMessageBox>
#include <KPushButton>
#include <KDebug>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/Connection>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/AccountSet>

#include <KTp/actions.h>

namespace KTp {

struct KTP_NO_EXPORT StartChatDialog::Private
{
    Private() :
        ui(new Ui::StartChatDialog),
        acceptInProgress(false)
    {}

    Ui::StartChatDialog *ui;
    bool acceptInProgress;
};

StartChatDialog::StartChatDialog(const Tp::AccountManagerPtr &accountManager, QWidget *parent) :
    KDialog(parent),
    d(new Private)
{
    setWindowTitle(i18n("Start a chat"));
    setWindowIcon(QIcon::fromTheme(QLatin1String("telepathy-kde")));

    QWidget *widget = new QWidget(this);
    d->ui->setupUi(widget);
    setMainWidget(widget);

    d->ui->accountCombo->setAccountSet(accountManager->onlineAccounts());

    d->ui->screenNameLineEdit->setFocus();
}

StartChatDialog::~StartChatDialog()
{
    delete d->ui;
    delete d;
}

void StartChatDialog::accept()
{
    Tp::AccountPtr account = d->ui->accountCombo->currentAccount();
    const QString contactIdentifier = d->ui->screenNameLineEdit->text();
    if (account.isNull()) {
        KMessageBox::sorry(this, i18n("No account selected."));
    } else if (account->connection().isNull()) {
        KMessageBox::sorry(this, i18n("The requested account has been disconnected "
                                      "and so a chat could not be initiated."));
    } else if (contactIdentifier.isEmpty()) {
        KMessageBox::sorry(this, i18n("You did not specify the name of the contact to start a chat with."));
    } else {
        Tp::PendingChannelRequest *op = KTp::Actions::startChat(account, contactIdentifier, true);
        connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(_k_onStartChatFinished(Tp::PendingOperation*)));

        setInProgress(true);
    }
}

void StartChatDialog::closeEvent(QCloseEvent *e)
{
    // ignore close event if we are in the middle of an operation
    if (!d->acceptInProgress) {
        KDialog::closeEvent(e);
    }
}

void StartChatDialog::_k_onStartChatFinished(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Failed to start a text channel with the contact for the given identifier"
                   << op->errorName() << op->errorMessage();
        KMessageBox::sorry(this, i18n("Failed to start a chat with the contact."));
        setInProgress(false);
    } else {
        QDialog::accept();
    }
}

void StartChatDialog::setInProgress(bool inProgress)
{
    d->acceptInProgress = inProgress;
    mainWidget()->setEnabled(!inProgress);
    button(KDialog::Ok)->setEnabled(!inProgress);
    button(KDialog::Cancel)->setEnabled(!inProgress);
}

} //namespace KTp

#include "start-chat-dialog.moc"
