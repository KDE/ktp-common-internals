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

#include "handler.h"

Handler::Handler(const QObject* parent):
    QObject(parent),
    AbstractClientHandler()
{
}

Handler::~Handler()
{
}

bool Handler::bypassApproval()
{
}

void Handler::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                             const Tp::AccountPtr &account,
                             const Tp::ConnectionPtr &connection,
                             const QList<Tp::ChannelPtr> &channels,
                             const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                             const QDateTime &userActionTime,
                             const Tp::AbstractClientHandler::HandlerInfo &handlerInfo)
{
    
}



