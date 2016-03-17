/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>

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


#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H

#include <QObject>
#include <QList>
#include <QLoggingCategory>
#include <KPluginInfo>

#include <KTp/message.h>
#include <KTp/outgoing-message.h>

#include <KTp/ktpcommoninternals_export.h>
#include <KTp/abstract-message-filter.h>

Q_DECLARE_LOGGING_CATEGORY(KTP_MESSAGEPROCESSOR)

namespace Tp
{
class ReceivedMessage;
class Message;
}


namespace KTp
{

class AbstractMessageFilter;

//each thing that displays message will have an instance of this
class KTPCOMMONINTERNALS_EXPORT MessageProcessor : public QObject
{
    Q_OBJECT

  public:
    static MessageProcessor* instance();
    ~MessageProcessor();

    //text-ui will call this somewhere when creating the template
    QString header();

    //text-ui will call this somewhere in handleIncommingMessage just before displaying it
    KTp::Message processIncomingMessage(const Tp::Message &message, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel);
    KTp::Message processIncomingMessage(const Tp::ReceivedMessage &message, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel);

    KTp::OutgoingMessage processOutgoingMessage(const QString &messageText, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel);

    KTp::Message processIncomingMessage(KTp::Message message, const KTp::MessageContext &context);
  protected:
    explicit MessageProcessor();

  private:
    class Private;
    Private * const d;

    friend class MessageFilterConfigManager;
};

}

#endif // MESSAGE_PROCESSOR_H
