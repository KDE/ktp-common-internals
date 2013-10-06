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

#include "text-channel-watcher-proxy-model.h"

#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/Types>
#include <TelepathyQt/ReceivedMessage>

#include <KTp/types.h>
#include <KTp/contact.h>
#include <KTp/message.h>


inline Tp::ChannelClassSpecList channelClasses() {
    return Tp::ChannelClassSpecList() << Tp::ChannelClassSpec::textChat();
}

class ChannelWatcher : public QObject, public Tp::RefCounted
{
    Q_OBJECT
public:
    ChannelWatcher(const QPersistentModelIndex &index, const Tp::TextChannelPtr &channel, QObject *parent=0);
    int unreadMessageCount() const;
    QString lastMessage() const;
    KTp::Message::MessageDirection lastMessageDirection() const;
    QPersistentModelIndex modelIndex() const;
Q_SIGNALS:
    void messagesChanged();
    void invalidated();
private Q_SLOTS:
    void onMessageReceived(const Tp::ReceivedMessage &message);
    void onMessageSent(const Tp::Message &message);
private:
    QPersistentModelIndex m_index;
    Tp::TextChannelPtr m_channel;
    QString m_lastMessage;
    KTp::Message::MessageDirection m_lastMessageDirection;
};

typedef Tp::SharedPtr<ChannelWatcher> ChannelWatcherPtr;

ChannelWatcher::ChannelWatcher(const QPersistentModelIndex &index, const Tp::TextChannelPtr &channel, QObject *parent):
    QObject(parent),
    m_index(index),
    m_channel(channel),
    m_lastMessageDirection(KTp::Message::LocalToRemote)
{
    connect(channel.data(), SIGNAL(pendingMessageRemoved(Tp::ReceivedMessage)), SIGNAL(messagesChanged()));
    connect(channel.data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)), SIGNAL(invalidated()));

    connect(channel.data(), SIGNAL(messageReceived(Tp::ReceivedMessage)), SLOT(onMessageReceived(Tp::ReceivedMessage)));
    connect(channel.data(), SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)), SLOT(onMessageSent(Tp::Message)));

    //trigger an update to the contact straight away
    QTimer::singleShot(0, this, SIGNAL(messagesChanged()));
}

QPersistentModelIndex ChannelWatcher::modelIndex() const
{
    return m_index;
}

int ChannelWatcher::unreadMessageCount() const
{
    return m_channel->messageQueue().size();
}

QString ChannelWatcher::lastMessage() const
{
    return m_lastMessage;
}

KTp::Message::MessageDirection ChannelWatcher::lastMessageDirection() const
{
    return m_lastMessageDirection;
}

void ChannelWatcher::onMessageReceived(const Tp::ReceivedMessage &message)
{
    if (!message.isDeliveryReport()) {
        m_lastMessage = message.text();
        m_lastMessageDirection = KTp::Message::RemoteToLocal;
        Q_EMIT messagesChanged();
    }
}

void ChannelWatcher::onMessageSent(const Tp::Message &message)
{
    m_lastMessage = message.text();
    m_lastMessageDirection = KTp::Message::LocalToRemote;
    Q_EMIT messagesChanged();
}

namespace KTp {

class TextChannelWatcherProxyModel::Private {
public:
    QHash<KTp::ContactPtr, ChannelWatcherPtr> currentChannels;
};

} //namespace




KTp::TextChannelWatcherProxyModel::TextChannelWatcherProxyModel(QObject *parent) :
    QIdentityProxyModel(parent),
    Tp::AbstractClientObserver(channelClasses(), true),
    d(new TextChannelWatcherProxyModel::Private)
{
}

KTp::TextChannelWatcherProxyModel::~TextChannelWatcherProxyModel()
{
    delete d;
}

void KTp::TextChannelWatcherProxyModel::observeChannels(const Tp::MethodInvocationContextPtr<> &context, const Tp::AccountPtr &account, const Tp::ConnectionPtr &connection, const QList<Tp::ChannelPtr> &channels, const Tp::ChannelDispatchOperationPtr &dispatchOperation, const QList<Tp::ChannelRequestPtr> &requestsSatisfied, const Tp::AbstractClientObserver::ObserverInfo &observerInfo)
{
    Q_UNUSED(context)
    Q_UNUSED(account)
    Q_UNUSED(connection)
    Q_UNUSED(dispatchOperation)
    Q_UNUSED(requestsSatisfied)
    Q_UNUSED(observerInfo)

    if (!sourceModel()) {
        return;
    }

    Q_FOREACH(const Tp::ChannelPtr & channel, channels) {
        Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (textChannel) {
            KTp::ContactPtr targetContact = KTp::ContactPtr::qObjectCast(textChannel->targetContact());

            //skip group chats and situations where we don't have a single contact to mark messages for
            if (targetContact.isNull()) {
                continue;
            }

            //if it's not in our source model, ignore the channel
            QModelIndexList matchedContacts = sourceModel()->match(QModelIndex(sourceModel()->index(0,0)), KTp::ContactRole, QVariant::fromValue(targetContact));
            if (matchedContacts.size() !=1) {
                continue;
            }

            QPersistentModelIndex contactIndex(matchedContacts[0]);

            ChannelWatcherPtr watcher = ChannelWatcherPtr(new ChannelWatcher(contactIndex, textChannel));
            d->currentChannels[targetContact] = watcher;

            connect(watcher.data(), SIGNAL(messagesChanged()), SLOT(onChannelMessagesChanged()));
        }
    }
}

QVariant KTp::TextChannelWatcherProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    const QModelIndex sourceIndex = mapToSource(proxyIndex);
    if (role == KTp::ContactHasTextChannelRole) {
        KTp::ContactPtr contact = sourceIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
        if (contact) {
            if (d->currentChannels.contains(contact)) {
                return true;
            }
        }
        return false;
    }

    if (role == KTp::ContactUnreadMessageCountRole) {
        KTp::ContactPtr contact = sourceIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
        if (contact) {
            if (d->currentChannels.contains(contact)) {
                return d->currentChannels[contact]->unreadMessageCount();
            }
        }
        return 0;
    }
    if (role == KTp::ContactLastMessageRole) {
        KTp::ContactPtr contact = sourceIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
        if (contact) {
            if (d->currentChannels.contains(contact)) {
                return d->currentChannels[contact]->lastMessage();
            }
        }
        return QString();
    }
    if (role == KTp::ContactLastMessageDirectionRole) {
        KTp::ContactPtr contact = sourceIndex.data(KTp::ContactRole).value<KTp::ContactPtr>();
        if (contact) {
            if (d->currentChannels.contains(contact)) {
                return d->currentChannels[contact]->lastMessageDirection();
            }
        }
        return KTp::Message::LocalToRemote;
    }

    return sourceIndex.data(role);
}

void KTp::TextChannelWatcherProxyModel::onChannelMessagesChanged()
{
    ChannelWatcher* watcher = qobject_cast<ChannelWatcher*>(sender());
    Q_ASSERT(watcher);
    const QModelIndex &index = mapFromSource(watcher->modelIndex());
    dataChanged(index, index);
}

void KTp::TextChannelWatcherProxyModel::onChannelInvalidated()
{
    ChannelWatcher* watcher = qobject_cast<ChannelWatcher*>(sender());
    Q_ASSERT(watcher);
    const QModelIndex &index = mapFromSource(watcher->modelIndex());
    const KTp::ContactPtr &contact = index.data(KTp::ContactRole).value<KTp::ContactPtr>();

    d->currentChannels.remove(contact);
    dataChanged(index, index);
}

#include "text-channel-watcher-proxy-model.moc"
#include "moc_text-channel-watcher-proxy-model.cpp"
