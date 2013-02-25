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


#ifndef TELEPATHY_TEXT_OBSERVER_H
#define TELEPATHY_TEXT_OBSERVER_H

#include "conversations-model.h"
#include "KTp/types.h"

#include <TelepathyQt/AbstractClientHandler>

class TelepathyTextObserver : public QObject
{
    Q_OBJECT

Q_PROPERTY(ConversationsModel *conversations READ conversationModel CONSTANT)
Q_PROPERTY(Tp::AccountManagerPtr accountManager READ accountManager CONSTANT)

  public:
    TelepathyTextObserver(QObject *parent = 0);
    ~TelepathyTextObserver();

    ConversationsModel* conversationModel() const;
    Tp::AccountManagerPtr accountManager() const;

  private:
    Tp::SharedPtr<ConversationsModel> m_handler;
    Tp::ClientRegistrarPtr m_registrar;
    Tp::AccountManagerPtr m_accountManager;
};

#endif // CONVERSATION_WATCHER_H
