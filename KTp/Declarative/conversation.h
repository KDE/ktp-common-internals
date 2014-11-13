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


#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <QObject>
#include <QIcon>
#include <TelepathyQt/Account>
#include <TelepathyQt/TextChannel>

#include "messages-model.h"

class MessagesModel;
class Conversation : public QObject
{
    Q_OBJECT

    Q_PROPERTY(MessagesModel *messages READ messages CONSTANT)
    Q_PROPERTY(bool valid READ isValid NOTIFY validityChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QIcon presenceIcon READ presenceIcon NOTIFY presenceIconChanged)
    Q_PROPERTY(QIcon avatar READ avatar NOTIFY avatarChanged)
    Q_PROPERTY(Tp::AccountPtr account READ account CONSTANT)
    Q_PROPERTY(KTp::ContactPtr targetContact READ targetContact CONSTANT)

public:
    Conversation(const Tp::TextChannelPtr &channel, const Tp::AccountPtr &account, QObject *parent = 0);
    Conversation(QObject *parent = 0);
    virtual ~Conversation();

    void setTextChannel(const Tp::TextChannelPtr &channel);
    Tp::TextChannelPtr textChannel() const;

    MessagesModel* messages() const;
    QString title() const;
    QIcon presenceIcon() const;
    QIcon avatar() const;

    /**
     * Target contact of this conversation. May be null if conversation is a group chat.
     */
    KTp::ContactPtr targetContact() const;
    Tp::AccountPtr account() const;

    bool isValid();

Q_SIGNALS:
    void validityChanged(bool isValid);
    void avatarChanged();
    void titleChanged();
    void presenceIconChanged();
    void conversationCloseRequested();

public Q_SLOTS:
    void delegateToProperClient();
    void requestClose();
    void updateTextChanged(const QString &message);

private Q_SLOTS:
    void onChannelInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage);
    void onAccountConnectionChanged(const Tp::ConnectionPtr &connection);
    void onCreateChannelFinished(Tp::PendingOperation *op);
    void onChatPausedTimerExpired();

private:
    class ConversationPrivate;
    ConversationPrivate *d;
};

Q_DECLARE_METATYPE(Conversation*)

#endif // CONVERSATION_H
