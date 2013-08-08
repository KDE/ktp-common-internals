/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "observer.h"
#include "logger.h"

#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/Channel>

#include <KDE/KDebug>

Observer::Observer(QObject* parent):
    QObject(parent),
    AbstractClientObserver(Tp::ChannelClassSpecList()
                << Tp::ChannelClassSpec::textChat()
                << Tp::ChannelClassSpec::textChatroom())
{
}

Observer::~Observer()
{
}

void Observer::observeChannels(const Tp::MethodInvocationContextPtr<> &context,
                               const Tp::AccountPtr &account,
                               const Tp::ConnectionPtr &connection,
                               const QList<Tp::ChannelPtr> &channels,
                               const Tp::ChannelDispatchOperationPtr &dispatchOperation,
                               const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                               const Tp::AbstractClientObserver::ObserverInfo &observerInfo)
{
    Q_FOREACH (const Tp::ChannelPtr &channel, channels) {
        new Logger(account, connection, channel);
    }

    context->setFinished();
}
