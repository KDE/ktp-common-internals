/*
    Copyright (C) 2011  Lasath Fernando <kde@lasath.org>

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


#include "conversation.h"
#include "messages-model.h"

#include <TelepathyQt/TextChannel>
#include <KDebug>
#include "conversation-target.h"

#include "channel-delegator.h"

class Conversation::ConversationPrivate
{
  public:
    MessagesModel *messages;
    ConversationTarget *target;
    //stores if the conversation has been delegated to another client and we are only observing the channel
    //and not handling it.
    bool delegated;
    bool valid;
    Tp::AccountPtr account;
};

Conversation::Conversation(const Tp::TextChannelPtr &channel,
                           const Tp::AccountPtr &account,
                           QObject *parent) :
        QObject(parent),
        d (new ConversationPrivate)
{
    kDebug();

    d->account = account;

    d->messages = new MessagesModel(account, this);
    d->messages->setTextChannel(channel);

    d->target = new ConversationTarget(account, KTp::ContactPtr::qObjectCast(channel->targetContact()), this);

    d->valid = channel->isValid();
    d->delegated = false;

    connect(channel.data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(onChannelInvalidated(Tp::DBusProxy*,QString,QString)));
}

Conversation::Conversation(QObject *parent) : QObject(parent)
{
    kError() << "Conversation should not be created directly. Use ConversationWatcher instead.";
    Q_ASSERT(false);
}

MessagesModel* Conversation::messages() const
{
    return d->messages;
}

ConversationTarget* Conversation::target() const
{
    return d->target;
}

bool Conversation::isValid()
{
    return d->valid;
}

void Conversation::onChannelInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage)
{
    kDebug() << proxy << errorName << ":" << errorMessage;

    d->valid = false;

    Q_EMIT validityChanged(d->valid);
}

void Conversation::delegateToProperClient()
{
    ChannelDelegator::delegateChannel(d->account, d->messages->textChannel());
    d->delegated = true;
    Q_EMIT conversationDelegated();
}

void Conversation::requestClose()
{
    kDebug();
    //if we are not handling the channel do nothing.
    if (!d->delegated) {
        d->messages->textChannel()->requestClose();
    }
}

Conversation::~Conversation()
{
    kDebug();
    requestClose();
    delete d;
}
