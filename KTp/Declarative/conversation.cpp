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
#include <TelepathyQt/Account>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingChannel>

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
    connect(d->account.data(), SIGNAL(connectionChanged(Tp::ConnectionPtr)), SLOT(onAccountConnectionChanged(Tp::ConnectionPtr)));

    d->messages = new MessagesModel(account, this);
    setTextChannel(channel);
    d->target = new ConversationTarget(account, KTp::ContactPtr::qObjectCast(channel->targetContact()), this);

    d->delegated = false;

}

Conversation::Conversation(QObject *parent) : QObject(parent)
{
    kError() << "Conversation should not be created directly. Use ConversationWatcher instead.";
    Q_ASSERT(false);
}

void Conversation::setTextChannel(const Tp::TextChannelPtr& channel)
{
    if (d->messages->textChannel() != channel) {
        d->messages->setTextChannel(channel);
        d->valid = channel->isValid();
        connect(channel.data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                SLOT(onChannelInvalidated(Tp::DBusProxy*,QString,QString)));
        Q_EMIT validityChanged(d->valid);
    }
}

Tp::TextChannelPtr Conversation::textChannel() const
{
    return d->messages->textChannel();
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

void Conversation::onAccountConnectionChanged(const Tp::ConnectionPtr& connection)
{
    //if we have reconnected and we were handling the channel
    if (connection && ! d->delegated) {

        //general convention is to never use ensureAndHandle when we already have a client registrar
        //ensureAndHandle will implicity create a new temporary client registrar which is a waste
        //it's also more code to get the new channel

        //However, we cannot use use ensureChannel as normal because without being able to pass a preferredHandler
        //we need a preferredHandler so that this handler is the one that ends up with the channel if multi handlers are active
        //we do not know the name that this handler is currently registered with
        Tp::PendingChannel *pendingChannel = d->account->ensureAndHandleTextChat(textChannel()->targetId());
        connect(pendingChannel, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onCreateChannelFinished(Tp::PendingOperation*)));
    }
}

void Conversation::onCreateChannelFinished(Tp::PendingOperation* op)
{
    Tp::PendingChannel *pendingChannelOp = qobject_cast<Tp::PendingChannel*>(op);
    Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(pendingChannelOp->channel());
    if (textChannel) {
        setTextChannel(textChannel);
    }
}

void Conversation::delegateToProperClient()
{
    ChannelDelegator::delegateChannel(d->account, d->messages->textChannel());
    d->delegated = true;
    Q_EMIT conversationCloseRequested();
}

void Conversation::requestClose()
{
    kDebug();

    //removing from the model will delete this object closing the channel
    Q_EMIT conversationCloseRequested();
}

Conversation::~Conversation()
{
    kDebug();
    //if we are not handling the channel do nothing.
    if (!d->delegated) {
        d->messages->textChannel()->requestClose();
    }
    delete d;
}
