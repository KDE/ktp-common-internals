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


#include "conversation-queue-manager.h"
#include <KDebug>

class ConversationQueueManager::ConversationQueManagerPrivate
{
  public:
    QList<Queueable*> queue;
    KAction *gloablAction;
};

void Queueable::enqueueSelf()
{
    m_queueManager->enqueue(this);
}

void Queueable::removeSelfFromQueue()
{
    m_queueManager->remove(this);
}

Queueable::~Queueable()
{
}

Queueable::Queueable(ConversationQueueManager *queue)
    : m_queueManager(queue)
{
    if(!queue) {
        m_queueManager = ConversationQueueManager::instance();
    }
}

ConversationQueueManager *ConversationQueueManager::instance()
{
    static ConversationQueueManager *m_instance = 0;

    if(!m_instance) {
        m_instance = new ConversationQueueManager();
    }

    return m_instance;
}

ConversationQueueManager::ConversationQueueManager(QObject *parent):
    QObject(parent),
    d(new ConversationQueManagerPrivate)
{
    kDebug();

    //FIXME: think of a good name for this. What did Kopete call it?
    d->gloablAction = new KAction(this);
    d->gloablAction->setObjectName(QLatin1String("next-unread-conversation"));
    d->gloablAction->setGlobalShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_I)/*, KAction::ActiveShortcut | KAction::DefaultShortcut, KAction::NoAutoloading*/);

    connect(d->gloablAction, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), SLOT(dequeueNext()));
}

void ConversationQueueManager::dequeueNext()
{
    kDebug();

    if(!d->queue.isEmpty()) {
        d->queue.takeLast()->selfDequeued();
    }
}

void ConversationQueueManager::enqueue(Queueable *item)
{
    if(!d->queue.contains(item)) {
        d->queue.append(item);
    }
}

void ConversationQueueManager::remove(Queueable *item)
{
    if(d->queue.contains(item)) {
        d->queue.removeAll(item);
    }
}

ConversationQueueManager::~ConversationQueueManager()
{
    delete d;
}
