/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "test-session.h"

namespace OTR
{

    TestSession::TestSession(const SessionContext &context, Manager *parent)
        : Session(context, parent)
    {
    }

    void TestSession::handleMessage(const Message &message)
    {
        if(message.isOTRevent()) {
            eventQueue.push_back(message);
        } else {
            mesQueue.push_back(message);
        }
    }

    int TestSession::recipientStatus() const
    {
        // user is online
        return 1;
    }

    unsigned int TestSession::maxMessageSize() const
    {
        return maxSize;
    }
}
