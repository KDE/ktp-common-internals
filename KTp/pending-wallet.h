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


#ifndef PENDINGWALLETOPERATION_H
#define PENDINGWALLETOPERATION_H

#include <TelepathyQt/PendingOperation>

#include "wallet-interface.h"
#include <KTp/ktpcommoninternals_export.h>

namespace KTp
{
class PendingWalletPrivate;

class KTPCOMMONINTERNALS_EXPORT PendingWallet : public Tp::PendingOperation
{
    Q_OBJECT
public:
    friend class WalletInterface;
    KTp::WalletInterface *walletInterface() const;
private:
    PendingWallet(KTp::WalletInterface *wallet);
    virtual ~PendingWallet();
    KTp::PendingWalletPrivate *d;
};

}
#endif // PENDINGWALLETOPERATION_H
