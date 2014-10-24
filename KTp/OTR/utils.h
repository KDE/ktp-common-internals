/*
    Copyright (C) 2014  Marcin Ziemiński   <zieminn@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OTR_UTILS_HEADER
#define OTR_UTILS_HEADER

#include "ktpotr_export.h"

#include <TelepathyQt/PendingVariant>
#include <TelepathyQt/Types>
#include <TelepathyQt/ReceivedMessage>
#include <TelepathyQt/TextChannel>

namespace KTp {
namespace Utils {

    /** Extracts pending-messages-ids from message list */
    KTPOTR_EXPORT Tp::UIntList getPendingMessagesIDs(const QList<Tp::ReceivedMessage> &messageQueue);

    /** Extracts pending-mesage-id from message */
    KTPOTR_EXPORT uint getId(const Tp::MessagePartList &message);

    /** Returns an object path for the otr proxy channel given a text channel it corresponds to */
    KTPOTR_EXPORT QString getOtrProxyObjectPathFor(const Tp::TextChannelPtr &textChannel);

    /** Returns true if message is generated internally by OTR implementation */
    KTPOTR_EXPORT bool isOtrEvent(const Tp::ReceivedMessage &message);

    /** Returns true if text is an OTR protocol message */
    KTPOTR_EXPORT bool isOtrMessage(const QString &text);

    /** Returns notification for a user assuming that the message is an otr event */
    KTPOTR_EXPORT QString processOtrMessage(const Tp::ReceivedMessage &message);
}
}

#endif
