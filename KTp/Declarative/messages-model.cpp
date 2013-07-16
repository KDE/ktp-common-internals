/*
    Copyright (C) 2011  Lasath Fernando <kde@lasath.org>
    Copyright (C) 2013  Lasath Fernando <davidedmundson@kde.org>

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


#include "messages-model.h"

#include <KDebug>
#include <KLocalizedString>

#include <TelepathyQt/ReceivedMessage>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Account>

#include "KTp/message-processor.h"

class MessagesModel::MessagesModelPrivate
{
  public:
    Tp::TextChannelPtr textChannel;
    Tp::AccountPtr account;
    QList<KTp::Message> messages;
    bool visible;
};

MessagesModel::MessagesModel(const Tp::AccountPtr &account, QObject *parent) :
        QAbstractListModel(parent),
        d(new MessagesModelPrivate)
{
    kDebug();

    QHash<int, QByteArray> roles;
    roles[TextRole] = "text";
    roles[TimeRole] = "time";
    roles[TypeRole] = "type";
    roles[SenderIdRole] = "senderId";
    roles[SenderAliasRole] = "senderAlias";
    roles[SenderAvatarRole] = "senderAvatar";
    setRoleNames(roles);

    d->account = account;
    d->visible = false;
}

Tp::TextChannelPtr MessagesModel::textChannel() const
{
    return d->textChannel;
}

bool MessagesModel::verifyPendingOperation(Tp::PendingOperation *op)
{
    bool operationSucceeded = true;

    if (op->isError()) {
        kWarning() << op->errorName() << "+" << op->errorMessage();
        operationSucceeded = false;
    }

    return operationSucceeded;
}

void MessagesModel::setupChannelSignals(const Tp::TextChannelPtr &channel)
{
    connect(channel.data(),
            SIGNAL(messageReceived(Tp::ReceivedMessage)),
            SLOT(onMessageReceived(Tp::ReceivedMessage)));
    connect(channel.data(),
            SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
            SLOT(onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString)));
    connect(channel.data(),
            SIGNAL(pendingMessageRemoved(Tp::ReceivedMessage)),
            SLOT(onPendingMessageRemoved()));
}

void MessagesModel::setTextChannel(const Tp::TextChannelPtr &channel)
{
    Q_ASSERT(channel != d->textChannel);
    kDebug();
    setupChannelSignals(channel);

    if (d->textChannel) {
        removeChannelSignals(d->textChannel);
    }

    d->textChannel = channel;

    QList<Tp::ReceivedMessage> messageQueue = channel->messageQueue();
    Q_FOREACH(const Tp::ReceivedMessage &message, messageQueue) {
        bool messageAlreadyInModel = false;
        Q_FOREACH(const KTp::Message &current, d->messages) {
            //FIXME: docs say messageToken can return an empty string. What to do if that happens?
            //Tp::Message has an == operator. maybe I can use that?
            if (current.token() == message.messageToken()) {
                messageAlreadyInModel = true;
                break;
            }
        }
        if (!messageAlreadyInModel) {
            onMessageReceived(message);
        }
    }
}

void MessagesModel::onMessageReceived(const Tp::ReceivedMessage &message)
{
    int unreadCount = d->textChannel->messageQueue().size();
    kDebug() << "unreadMessagesCount =" << unreadCount;
    kDebug() << "text =" << message.text();
    kDebug() << "messageType = " << message.messageType();
    kDebug() << "messageToken =" << message.messageToken();

    //delivery reports do not contain message text, everything else does.
    //simply ack these straight away

    //TODO search through d->messages() for messages with identical messageToken and update sending state as appropriate

    if (message.messageType() == Tp::ChannelTextMessageTypeDeliveryReport) {
        d->textChannel->acknowledge(QList<Tp::ReceivedMessage>() << message);
    } else {
        int length = rowCount();
        beginInsertRows(QModelIndex(), length, length);


        d->messages.append(KTp::MessageProcessor::instance()->processIncomingMessage(
                               message, d->account, d->textChannel));

        endInsertRows();

        if (d->visible) {
            acknowledgeAllMessages();
        } else {
            Q_EMIT unreadCountChanged(unreadCount);
        }
    }
}

void MessagesModel::onMessageSent(const Tp::Message &message, Tp::MessageSendingFlags flags, const QString &messageToken)
{
    Q_UNUSED(flags);
    Q_UNUSED(messageToken);

    int length = rowCount();
    beginInsertRows(QModelIndex(), length, length);
    kDebug() << "text =" << message.text();

    d->messages.append(KTp::MessageProcessor::instance()->processIncomingMessage(
                           message, d->account, d->textChannel));
    endInsertRows();
}

void MessagesModel::onPendingMessageRemoved()
{
    Q_EMIT unreadCountChanged(unreadCount());
}

QVariant MessagesModel::data(const QModelIndex &index, int role) const
{
    QVariant result;

    if (index.isValid()) {
        const KTp::Message message = d->messages[index.row()];

        switch (role) {
        case TextRole:
            result = message.finalizedMessage();
            break;
        case TypeRole:
            if (message.type() == Tp::ChannelTextMessageTypeAction) {
                result = MessageTypeAction;
            } else {
                if (message .direction() == KTp::Message::LocalToRemote) {
                    result = MessageTypeOutgoing;
                } else {
                    result = MessageTypeIncoming;
                }
            }
            break;
        case TimeRole:
            result = message.time();
            break;
        case SenderIdRole:
            result = message.senderId();
            break;
        case SenderAliasRole:
            result = message.senderAlias();
            break;
        case SenderAvatarRole:
            if (message.sender()) {
                result = QVariant::fromValue(message.sender()->avatarPixmap());
            }
            break;
        };
    } else {
        kError() << "Attempting to access data at invalid index (" << index << ")";
    }

    return result;
}

int MessagesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->messages.size();
}

void MessagesModel::sendNewMessage(const QString &message)
{
    if (message.isEmpty()) {
        kWarning() << "Attempting to send empty string";
    } else {
        Tp::PendingOperation *op;
        QString modifiedMessage = message;
        if (d->textChannel->supportsMessageType(Tp::ChannelTextMessageTypeAction)
                && modifiedMessage.startsWith(QLatin1String("/me "))) {
            //remove "/me " from the start of the message
            modifiedMessage.remove(0,4);
            op = d->textChannel->send(modifiedMessage, Tp::ChannelTextMessageTypeAction);
        } else {
            op = d->textChannel->send(modifiedMessage);
        }
        connect(op,
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(verifyPendingOperation(Tp::PendingOperation*)));
    }
}

void MessagesModel::removeChannelSignals(const Tp::TextChannelPtr &channel)
{
    QObject::disconnect(channel.data(),
                        SIGNAL(messageReceived(Tp::ReceivedMessage)),
                        this,
                        SLOT(onMessageReceived(Tp::ReceivedMessage))
                       );
    QObject::disconnect(channel.data(),
                        SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
                        this,
                        SLOT(onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString))
                       );
}

int MessagesModel::unreadCount() const
{
    return d->textChannel->messageQueue().size();
}

void MessagesModel::acknowledgeAllMessages()
{
    QList<Tp::ReceivedMessage> queue = d->textChannel->messageQueue();

    kDebug() << "Conversation Visible, Acknowledging " << queue.size() << " messages.";

    d->textChannel->acknowledge(queue);
    Q_EMIT unreadCountChanged(queue.size());
}

void MessagesModel::setVisibleToUser(bool visible)
{
    kDebug() << visible;

    if (d->visible != visible) {
        d->visible = visible;
        Q_EMIT visibleToUserChanged(d->visible);
    }

    if (visible) {
        acknowledgeAllMessages();
    }
}

bool MessagesModel::isVisibleToUser() const
{
    return d->visible;
}

MessagesModel::~MessagesModel()
{
    kDebug();
    delete d;
}

bool MessagesModel::shouldStartOpened() const
{
    return d->textChannel->isRequested();
}
