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

#ifndef KTP_MESSAGE_CONTEXT_H
#define KTP_MESSAGE_CONTEXT_H

#include <TelepathyQt/Account>
#include <TelepathyQt/TextChannel>

#include "ktp-export.h"

namespace KTp
{

class KTP_EXPORT MessageContext
{
public:
    MessageContext(const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel);

    virtual ~MessageContext();

    /** Account in which the message is sent
    */
    Tp::AccountPtr account() const;

    /** Channel in which the message was sent
     @warning this may be null, and should always be checked before use
    */
    Tp::TextChannelPtr channel() const;

private:
    class Private;
    Private *d;
};

}
#endif // MESSAGECONTEXT_H
