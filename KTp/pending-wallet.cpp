/*
 * Copyright (C) 2012 David Edmundson <kde@davidedmundson.co.uk>
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

#include "pending-wallet.h"

#include "wallet-interface.h"

namespace KTp {
class PendingWalletPrivate {
public:
    KTp::WalletInterface *walletInterface;
};
}

KTp::PendingWallet::PendingWallet(KTp::WalletInterface* walletInterface)
    :Tp::PendingOperation(Tp::SharedPtr<Tp::RefCounted>(0)),
      d( new KTp::PendingWalletPrivate())
{
    d->walletInterface = walletInterface;

    //if wallet is not enabled, or wallet is already opened, setFinished() immediately
    if (!walletInterface->wallet() || walletInterface->isOpen()) {
        setFinished();
    } else {
        connect(walletInterface->wallet(), SIGNAL(walletOpened(bool)), SLOT(setFinished()));
    }
}

KTp::PendingWallet::~PendingWallet()
{
    delete d;
}

KTp::WalletInterface *KTp::PendingWallet::walletInterface() const
{
    return d->walletInterface;
}

