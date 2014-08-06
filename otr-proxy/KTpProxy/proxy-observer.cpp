/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "proxy-observer.h"
#include "proxy-service.h"
#include "pending-curry-operation.h"

#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/Channel>
#include <TelepathyQt/PendingReady>

#include <KDebug>

// class to wait for all account fields to be ready
// (displayName is not properly initializted sometimes)
// and magic things happen
class PendingAccountReady : public PendingCurryOperation
{
    public:
        PendingAccountReady(
                Tp::PendingOperation *op,
                const Tp::AccountPtr &account,
                const QList<Tp::ChannelPtr> &channels,
                const Tp::MethodInvocationContextPtr<> &context,
                ProxyService *ps)
            : PendingCurryOperation(op, Tp::SharedPtr<Tp::RefCounted>::dynamicCast(account)),
            account(account),
            channels(channels),
            context(context),
            ps(ps)
        {
        }

        virtual void extract(Tp::PendingOperation *op)
        {
            Q_UNUSED(op);

            Q_FOREACH(const Tp::ChannelPtr &chan, channels) {
                ps->addChannel(chan, account);
            }
            context->setFinished();
        }

    private:
        Tp::AccountPtr account;
        QList<Tp::ChannelPtr> channels;
        Tp::MethodInvocationContextPtr<> context;
        ProxyService *ps;
};

ProxyObserver::ProxyObserver(ProxyService *ps)
    : Tp::AbstractClientObserver(Tp::ChannelClassSpecList() << Tp::ChannelClassSpec::textChat()),
    ps(ps)
{

}

ProxyObserver::~ProxyObserver() { }

void ProxyObserver::observeChannels(
        const Tp::MethodInvocationContextPtr<> &context,
        const Tp::AccountPtr &account,
        const Tp::ConnectionPtr &connection,
        const QList<Tp::ChannelPtr> &channels,
        const Tp::ChannelDispatchOperationPtr &dispatchOperation,
        const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
        const Tp::AbstractClientObserver::ObserverInfo &observerInfo)
{

    Q_UNUSED(connection);
    Q_UNUSED(dispatchOperation);
    Q_UNUSED(requestsSatisfied);
    Q_UNUSED(observerInfo);
    kDebug() << "Observed a channel";

    PendingAccountReady *ready = new PendingAccountReady(
            account->becomeReady(Tp::Features() << Tp::Account::FeatureCore),
            account, channels, context, ps);

    QObject::connect(ready, SIGNAL(finished(Tp::PendingOperation*)), ready, SLOT(deleteLater()));
}
