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

#include "otr-session.h"

extern "C" {
#include <libotr/privkey.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
}

namespace OTR
{
    UserStateBox::UserStateBox(OtrlUserState userState)
        : us(userState)
    {
        uint interval = otrl_message_poll_get_default_interval(us);
        QObject::connect(&periodicTimer, SIGNAL(timeout()), SLOT(otrlMessagePoll()));
        periodicTimer.start(interval * 1000);
    }

    UserStateBox::~UserStateBox()
    {
        otrl_userstate_free(us);
    }

    OtrlUserState UserStateBox::userState()
    {
        return us;
    }

    void UserStateBox::setInterval(uint interval)
    {
        if(interval) {
            periodicTimer.start(interval * 1000);
        } else {
            periodicTimer.stop();
        }
    }

    void UserStateBox::otrlMessagePoll()
    {
        otrl_message_poll(us, 0, 0);
    }

    Session::Session(const HandlerPtr &handler, UserStateBox *userstate, Manager *parent)
        : hd(handler),
        userstate(userstate),
        pr(parent)
    {
    }

    const HandlerPtr& Session::handler()
    {
        return hd;
    }

    UserStateBox* Session::userStateBox()
    {
        return userstate;
    }

    Manager* Session::parent()
    {
        return pr;
    }

} /* namespace OTR */
