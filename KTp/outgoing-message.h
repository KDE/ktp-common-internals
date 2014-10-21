/*
    Copyright (C) 2013  David Edmundson <kde@davidedmundson.co.uk>

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


#ifndef KTP_OUTGOING_MESSAGE_H
#define KTP_OUTGOING_MESSAGE_H

#include <KTp/ktpcommoninternals_export.h>
#include <KTp/message-context.h>

#include <TelepathyQt/Types>

#include <QSharedData>
#include <QSharedDataPointer>

namespace KTp
{

/*!
 * \par
 * An encapsualtion of a message that is to be sent out
 *
 * \author David Edmundson <kde@davidedmundson.co.uk>
 */
class KTPCOMMONINTERNALS_EXPORT OutgoingMessage
{
  public:
    OutgoingMessage(const KTp::OutgoingMessage &other);
    KTp::OutgoingMessage& operator=(const KTp::OutgoingMessage &other);
    virtual ~OutgoingMessage();

    /*! \brief The body of the message
     * \return the contents of the message as a plain string
     */
    QString text() const;

    void setText(const QString &text);

    void setType(Tp::ChannelTextMessageType type);

    Tp::ChannelTextMessageType type() const;


protected:
    explicit OutgoingMessage(const QString &messageText);

private:
    class Private;
    QSharedDataPointer<Private> d;
    friend class MessageProcessor;
};

}


#endif // KTP_PENDING_SEND_MESSAGE_H
