/*
    Copyright (C) 2011  Lasath Fernando <kde@lasath.org>
    Copyright (C) 2016  Martin Klapetek <mklapetek@kde.org>

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

#include "debug.h"

#include "channel-delegator.h"

class Conversation::ConversationPrivate
{
  public:
      ConversationPrivate()
      {
          messages = 0;
          delegated = false;
          valid = false;
          isGroupChat = false;
      }

    MessagesModel *messages;
    //stores if the conversation has been delegated to another client and we are only observing the channel
    //and not handling it.
    bool delegated;
    bool valid;
    Tp::AccountPtr account;
    QTimer *pausedStateTimer;
    // May be null for group chats.
    KTp::ContactPtr targetContact;
    bool isGroupChat;
};

Conversation::Conversation(const Tp::TextChannelPtr &channel,
                           const Tp::AccountPtr &account,
                           QObject *parent) :
        QObject(parent),
        d (new ConversationPrivate)
{
    qCDebug(KTP_DECLARATIVE);

    d->account = account;
    connect(d->account.data(), SIGNAL(connectionChanged(Tp::ConnectionPtr)), SLOT(onAccountConnectionChanged(Tp::ConnectionPtr)));

    d->messages = new MessagesModel(account, this);
    connect(d->messages, &MessagesModel::unreadCountChanged, this, &Conversation::unreadMessagesChanged);
    setTextChannel(channel);

    d->delegated = false;

    d->pausedStateTimer = new QTimer(this);
    d->pausedStateTimer->setSingleShot(true);
    connect(d->pausedStateTimer, SIGNAL(timeout()), this, SLOT(onChatPausedTimerExpired()));
}

Conversation::Conversation(QObject *parent)
    : QObject(parent),
      d(new ConversationPrivate)
{
}

void Conversation::setTextChannel(const Tp::TextChannelPtr &channel)
{
    if (!d->messages) {
        d->messages = new MessagesModel(d->account, this);
        connect(d->messages, &MessagesModel::unreadCountChanged, this, &Conversation::unreadMessagesChanged);
    }
    if (d->messages->textChannel() != channel) {
        d->messages->setTextChannel(channel);
        d->valid = channel->isValid();
        connect(channel.data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                SLOT(onChannelInvalidated(Tp::DBusProxy*,QString,QString)));

        if (channel->targetContact().isNull()) {
            d->isGroupChat = true;
        } else {
            d->isGroupChat = false;
            d->targetContact = KTp::ContactPtr::qObjectCast(channel->targetContact());

            connect(d->targetContact.constData(), SIGNAL(aliasChanged(QString)), SIGNAL(titleChanged()));
            connect(d->targetContact.constData(), SIGNAL(presenceChanged(Tp::Presence)), SIGNAL(presenceIconChanged()));
            connect(d->targetContact.constData(), SIGNAL(avatarDataChanged(Tp::AvatarData)), SIGNAL(avatarChanged()));
        }

        Q_EMIT avatarChanged();
        Q_EMIT titleChanged();
        Q_EMIT presenceIconChanged();
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

QString Conversation::title() const
{
    if (d->isGroupChat) {
        QString roomName = textChannel()->targetId();
        return roomName.left(roomName.indexOf(QLatin1Char('@')));
    } else if (!d->targetContact.isNull()) {
        return d->targetContact->alias();
    }

    return QString();
}

QIcon Conversation::presenceIcon() const
{
    if (d->isGroupChat) {
        return KTp::Presence(Tp::Presence::available()).icon();
    } else if (!d->targetContact.isNull()) {
        return KTp::Presence(d->targetContact->presence()).icon();
    }

    return QIcon();
}

QIcon Conversation::avatar() const
{
    if (d->isGroupChat) {
        return QIcon();
    } else {
        const QString path = d->targetContact->avatarData().fileName;
        QIcon icon;
        if (!path.isEmpty()) {
            icon = QIcon(path);
        }
        if (icon.availableSizes().isEmpty()) {
            icon = QIcon::fromTheme(QStringLiteral("im-user"));
        }
        return icon;
    }
}

KTp::ContactPtr Conversation::targetContact() const
{
    if (d->isGroupChat) {
        return KTp::ContactPtr();
    } else {
        return d->targetContact;
    }
}

Tp::AccountPtr Conversation::account() const
{
    return d->account;
}

bool Conversation::isValid() const
{
    return d->valid;
}

void Conversation::onChannelInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage)
{
    qCDebug(KTP_DECLARATIVE) << proxy << errorName << ":" << errorMessage;

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
    qCDebug(KTP_DECLARATIVE);

    //removing from the model will delete this object closing the channel
    Q_EMIT conversationCloseRequested();
}

void Conversation::updateTextChanged(const QString &message)
{
    if (!message.isEmpty()) {
        //if the timer is active, it means the user is continuously typing
        if (d->pausedStateTimer->isActive()) {
            //just restart the timer and don't spam with chat state changes
            d->pausedStateTimer->start(5000);
        } else {
            //if the user has just typed some text, set state to Composing and start the timer
            d->messages->textChannel()->requestChatState(Tp::ChannelChatStateComposing);
            d->pausedStateTimer->start(5000);
        }
    } else {
        //if the user typed no text/cleared the input field, set Active and stop the timer
        d->messages->textChannel()->requestChatState(Tp::ChannelChatStateActive);
        d->pausedStateTimer->stop();
    }
}

void Conversation::onChatPausedTimerExpired()
{
    d->messages->textChannel()->requestChatState(Tp::ChannelChatStatePaused);
}

Conversation::~Conversation()
{
    qCDebug(KTP_DECLARATIVE);
    //if we are not handling the channel do nothing.
    if (!d->delegated) {
        d->messages->textChannel()->requestClose();
    }
    delete d;
}

bool Conversation::hasUnreadMessages() const
{
    if (d->messages) {
        return d->messages->unreadCount() > 0;
    }

    return false;
}
