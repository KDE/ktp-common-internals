/*
* Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
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

#include "message-context.h"

namespace KTp {
class MessageContext::Private {
public:
    Tp::AccountPtr account;
    Tp::TextChannelPtr channel;
};
}

KTp::MessageContext::MessageContext(const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel) :
    d(new Private)
{
    d->account = account;
    d->channel = channel;
}

KTp::MessageContext::~MessageContext()
{
    delete d;
}

Tp::AccountPtr KTp::MessageContext::account() const
{
    return d->account;
}

Tp::TextChannelPtr KTp::MessageContext::channel() const
{
    return d->channel;
}

