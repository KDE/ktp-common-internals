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

#ifndef KTP_PROXY_OTR_TEST_SESSION_HEADER
#define KTP_PROXY_OTR_TEST_SESSION_HEADER

#include <KTpProxy/otr-session.h>
#include <KTpProxy/otr-message.h>

#include <QList>

namespace OTR
{
    class TestSession : public Session
    {
        public:
            TestSession(const SessionContext &context, Manager *parent);

            virtual void handleMessage(const Message &message);
            virtual int recipientStatus() const;
            virtual unsigned int maxMessageSize() const;

            uint maxSize = 1000000;
            QList<Message> mesQueue;
            QList<Message> eventQueue;
    };
}

#endif
