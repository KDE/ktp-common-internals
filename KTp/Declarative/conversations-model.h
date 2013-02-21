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


#ifndef CONVERSATIONS_MODEL_H
#define CONVERSATIONS_MODEL_H

#include <QAbstractListModel>

#include <TelepathyQt/AbstractClientApprover>
#include <KTp/contact.h>

class Conversation;

class ConversationsModel : public QAbstractListModel, public Tp::AbstractClientHandler
{
    Q_OBJECT
    Q_PROPERTY(int totalUnreadCount READ totalUnreadCount NOTIFY totalUnreadCountChanged)

  public:
    explicit ConversationsModel(QObject *parent);
    virtual ~ConversationsModel();

    virtual QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual int rowCount ( const QModelIndex &parent = QModelIndex() ) const;

    /** @returns the sum of all unread messages among all conversations */
    int totalUnreadCount() const;

    enum role {
        ConversationRole = Qt::UserRole
    };

    void handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                        const Tp::AccountPtr &account,
                        const Tp::ConnectionPtr &connection,
                        const QList<Tp::ChannelPtr> &channels,
                        const QList<Tp::ChannelRequestPtr> &channelRequests,
                        const QDateTime &userActionTime,
                        const HandlerInfo &handlerInfo);
    bool bypassApproval() const;

  public Q_SLOTS:
    void startChat(const Tp::AccountPtr &account, const KTp::ContactPtr &contact);
    int nextActiveConversation(int first);

  private:
    void removeConversation(Conversation* conversation);

    class ConversationsModelPrivate;
    ConversationsModelPrivate *d;

  private Q_SLOTS:
    void handleValidityChange(bool);
    void conversationDelegated();

  Q_SIGNALS:
      void totalUnreadCountChanged();
};

#endif // CONVERSATIONS_MODEL_H
