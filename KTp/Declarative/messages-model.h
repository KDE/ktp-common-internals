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


#ifndef MESSAGES_MODEL_H
#define MESSAGES_MODEL_H

#include <QAbstractItemModel>

#include <TelepathyQt/Types>
#include <TelepathyQt/ReceivedMessage>

#include <KTp/message.h>

class MessagesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(MessageType)
    Q_ENUMS(DeliveryStatus)
    Q_PROPERTY(bool visibleToUser READ isVisibleToUser WRITE setVisibleToUser NOTIFY visibleToUserChanged)
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY unreadCountChanged)
    Q_PROPERTY(bool shouldStartOpened READ shouldStartOpened CONSTANT)

  public:
    MessagesModel(const Tp::AccountPtr &account, QObject *parent = 0);
    virtual ~MessagesModel();

    enum Roles {
        TextRole = Qt::UserRole, //String
        TypeRole, //MessagesModel::MessageType (for now!)
        TimeRole, //QDateTime
        SenderIdRole, //string
        SenderAliasRole, //string
        SenderAvatarRole, //pixmap
        DeliveryStatusRole, //MessagesModel::DeliveryStatus
        DeliveryReportReceiveTimeRole //QDateTime
    };

    enum MessageType {
        MessageTypeIncoming,
        MessageTypeOutgoing,
        MessageTypeAction,
        MessageTypeNotice
    };

    enum DeliveryStatus {
        DeliveryStatusUnknown,
        DeliveryStatusDelivered,
        DeliveryStatusRead, // implies DeliveryStatusDelivered
        DeliveryStatusFailed
    };

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

    Tp::TextChannelPtr textChannel() const;
    void setTextChannel(const Tp::TextChannelPtr &channel);

    bool isVisibleToUser() const;
    void setVisibleToUser(bool visible);

    int  unreadCount() const;
    void acknowledgeAllMessages();

    bool shouldStartOpened() const;

  Q_SIGNALS:
    void visibleToUserChanged(bool visible);
    void unreadCountChanged(int unreadMesssagesCount);

  public Q_SLOTS:
    void sendNewMessage(const QString &message);

  private Q_SLOTS:
    void onMessageReceived(const Tp::ReceivedMessage &message);
    void onMessageSent(const Tp::Message &message, Tp::MessageSendingFlags flags, const QString &messageToken);
    void onPendingMessageRemoved();
    bool verifyPendingOperation(Tp::PendingOperation *op);
    void onHistoryFetched(const QList<KTp::Message> &messages);

  private:
    void setupChannelSignals(const Tp::TextChannelPtr &channel);
    void removeChannelSignals(const Tp::TextChannelPtr &channel);

    class MessagesModelPrivate;
    MessagesModelPrivate *d;
};

#endif // CONVERSATION_MODEL_H
