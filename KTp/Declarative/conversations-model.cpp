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


#include "conversations-model.h"
#include "conversation.h"
#include "messages-model.h"

#include "debug.h"

#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ClientRegistrar>

static inline Tp::ChannelClassSpecList channelClassList()
{
    return Tp::ChannelClassSpecList() << Tp::ChannelClassSpec::textChat();
}

class ConversationsModel::ConversationsModelPrivate
{
  public:
    QList<Conversation*> conversations;
    int activeChatIndex;
};

ConversationsModel::ConversationsModel(QObject *parent) :
    QAbstractListModel(parent),
    Tp::AbstractClientHandler(channelClassList()),
    d(new ConversationsModelPrivate)
{
    d->activeChatIndex = -1;
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(totalUnreadCountChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(totalUnreadCountChanged()));
}

ConversationsModel::~ConversationsModel()
{
    qDeleteAll(d->conversations);
    delete d;
}

QHash<int, QByteArray> ConversationsModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[ConversationRole] = "conversation";
    return roles;
}

QVariant ConversationsModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    if (index.isValid() && role == ConversationRole) {
        result = QVariant::fromValue<Conversation*>(d->conversations[index.row()]);
    }
    return result;
}

int ConversationsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->conversations.count();
}

void ConversationsModel::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                                        const Tp::AccountPtr &account,
                                        const Tp::ConnectionPtr &connection,
                                        const QList<Tp::ChannelPtr> &channels,
                                        const QList<Tp::ChannelRequestPtr> &channelRequests,
                                        const QDateTime &userActionTime,
                                        const HandlerInfo &handlerInfo)
{
    Q_UNUSED(connection);
    Q_UNUSED(handlerInfo);
    Q_UNUSED(userActionTime);

    bool handled = false;
    bool shouldDelegate = false;

    //check that the channel is of type text
    Tp::TextChannelPtr textChannel;
    Q_FOREACH (const Tp::ChannelPtr &channel, channels) {
        textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (textChannel) {
            break;
        }
    }

    Q_ASSERT(textChannel);

    //find the relevant channelRequest
    Q_FOREACH (const Tp::ChannelRequestPtr channelRequest, channelRequests) {
        qCDebug(KTP_DECLARATIVE) << channelRequest->hints().allHints();
        shouldDelegate = channelRequest->hints().hint(QLatin1String("org.freedesktop.Telepathy.ChannelRequest"),
                                                      QLatin1String("DelegateToPreferredHandler")).toBool();
    }

    //loop through all conversations checking for matches

    //if we are handling and we're not told to delegate it, update the text channel
    //if we are handling but should delegate, call delegate channel
    int i = 0;
    Q_FOREACH (Conversation *conversation, d->conversations) {
        if (conversation->textChannel()->targetId() == textChannel->targetId()
            && conversation->textChannel()->targetHandleType() == textChannel->targetHandleType())
        {
            if (!shouldDelegate) {
                conversation->setTextChannel(textChannel);
                //Update the active chat index to this channel
                d->activeChatIndex = i;
                Q_EMIT activeChatIndexChanged();
            } else {
                if (conversation->textChannel() == textChannel) {
                    conversation->delegateToProperClient();
                }
            }
            handled = true;
            break;
        }
        i++;
    }

    //if we are not handling channel already and should not delegate, add the conversation
    //if we not handling the channel but should delegate it, do nothing.
    if (!handled && !shouldDelegate) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        Conversation *newConvo = new Conversation(textChannel, account, this);
        connect(newConvo, SIGNAL(conversationCloseRequested()), SLOT(onConversationCloseRequested()));
        connect(newConvo->messages(), SIGNAL(unreadCountChanged(int)), SIGNAL(totalUnreadCountChanged()));
        d->conversations.append(newConvo);
        endInsertRows();

        //If this is a locally generated request or there is no active chat, the index of the newly inserted conversation is saved as the active chat
        //The model is reset to load the newly created chat channel
        if (textChannel->isRequested() || d->activeChatIndex == -1) {
            d->activeChatIndex = rowCount() - 1;
            Q_EMIT activeChatIndexChanged();
        }
        context->setFinished();
    }
}

bool ConversationsModel::bypassApproval() const
{
    return true;
}

void ConversationsModel::onConversationCloseRequested()
{
    removeConversation(qobject_cast<Conversation*>(QObject::sender()));
}

void ConversationsModel::removeConversation(Conversation* conv)
{
    int index = d->conversations.indexOf(conv);
    if (index != -1) {
        beginRemoveRows(QModelIndex(), index, index);
        d->conversations.removeAt(index);
        conv->deleteLater();
        endRemoveRows();
    } else {
        qWarning() << "attempting to delete non-existent conversation";
    }
}

int ConversationsModel::nextActiveConversation(int fromRow)
{
    if (d->conversations.isEmpty()) {
        return -1;
    }
    Q_ASSERT(qBound(0, fromRow, d->conversations.count() - 1) == fromRow);

    bool first = true; //let first be checked on the first loop
    for (int i = fromRow; i != fromRow || first; i = (i + 1) % d->conversations.count()) {
        if (d->conversations[i]->messages()->unreadCount() > 0) {
            return i;
        }
        first = false;
    }
    return -1;
}

int ConversationsModel::totalUnreadCount() const
{
    int ret = 0;
    Q_FOREACH(Conversation *c, d->conversations) {
        ret += c->messages()->unreadCount();
    }
    return ret;
}

int ConversationsModel::activeChatIndex() const
{
    return d->activeChatIndex;
}

void ConversationsModel::closeAllConversations()
{
    if (!d->conversations.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        d->conversations.clear();
        endRemoveRows();
        qDeleteAll(d->conversations);
    }
}
