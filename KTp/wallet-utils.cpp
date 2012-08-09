/*
    Copyright (C) 2012  David Edmundson <kde@davidedmundson.co.uk>

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

#include "wallet-utils.h"

#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/Account>

#include <KTp/wallet-interface.h>
#include <KTp/pending-wallet.h>

class SetAccountPasswordOp : public Tp::PendingOperation
{
    Q_OBJECT
public:
    explicit SetAccountPasswordOp(const Tp::AccountPtr &account, const QString &password);

    static Tp::PendingOperation * setAccountPassword(const Tp::AccountPtr &account, const QString &password);

private Q_SLOTS:
    void onWalletOpened(Tp::PendingOperation *op);
private:
    Tp::AccountPtr m_account;
    QString m_password;
};

class RemoveAccountPasswordOp : public Tp::PendingOperation
{
    Q_OBJECT
public:
    explicit RemoveAccountPasswordOp(const Tp::AccountPtr &account);

private Q_SLOTS:
    void onWalletOpened(Tp::PendingOperation *op);
private:
    Tp::AccountPtr m_account;
};


Tp::PendingOperation* KTp::WalletUtils::setAccountPassword(const Tp::AccountPtr &account, const QString &password)
{
    return new SetAccountPasswordOp(account, password);
}

Tp::PendingOperation* KTp::WalletUtils::removeAccountPassword(const Tp::AccountPtr &account)
{
    return new RemoveAccountPasswordOp(account);
}

SetAccountPasswordOp::SetAccountPasswordOp(const Tp::AccountPtr &account, const QString &password) :
    Tp::PendingOperation(account),
    m_account(account),
    m_password(password)
{
    connect(KTp::WalletInterface::openWallet(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onWalletOpened(Tp::PendingOperation*)));
}

void SetAccountPasswordOp::onWalletOpened(Tp::PendingOperation *op)
{
    KTp::PendingWallet *walletOp = qobject_cast<KTp::PendingWallet*>(op);
    Q_ASSERT(walletOp);

    KTp::WalletInterface *walletInterface = walletOp->walletInterface();

    //note deliberate using isNull, not isEmpty, as the password could be empty which is valid
    if (m_password.isNull()) {
        walletInterface->removePassword(m_account);
    } else {
        walletInterface->setPassword(m_account, m_password);
    }
}


RemoveAccountPasswordOp::RemoveAccountPasswordOp(const Tp::AccountPtr &account) :
    Tp::PendingOperation(account),
    m_account(account)
{
    connect(KTp::WalletInterface::openWallet(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onWalletOpened(Tp::PendingOperation*)));
}

void RemoveAccountPasswordOp::onWalletOpened(Tp::PendingOperation *op)
{
    KTp::PendingWallet *walletOp = qobject_cast<KTp::PendingWallet*>(op);
    Q_ASSERT(walletOp);

    KTp::WalletInterface *walletInterface = walletOp->walletInterface();
    walletInterface->removeAccount(m_account);
    setFinished();
}

#include "wallet-utils.moc"
