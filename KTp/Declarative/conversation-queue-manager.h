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


#ifndef CONVERSATION_QUEUE_MANAGER_H
#define CONVERSATION_QUEUE_MANAGER_H

#include <QtCore/QObject>
#include <KAction>

class ConversationQueueManager;
class Queueable
{
  friend class ConversationQueueManager;

  protected:
    Queueable(ConversationQueueManager *queue = 0);
    virtual ~Queueable();

    void enqueueSelf();
    void removeSelfFromQueue();
    virtual void selfDequeued() = 0;

  private:
    ConversationQueueManager *m_queueManager;
};


class ConversationQueueManager : public QObject
{
    Q_OBJECT

  public:
    static ConversationQueueManager* instance();
    void enqueue(Queueable *item);
    void remove(Queueable *item);

  public Q_SLOTS:
    void dequeueNext();

  private:
    explicit ConversationQueueManager(QObject *parent = 0);
    virtual ~ConversationQueueManager();

    class ConversationQueManagerPrivate;
    ConversationQueManagerPrivate *d;
};

#endif // CONVERSATION_QUEUE_MANAGER_H
