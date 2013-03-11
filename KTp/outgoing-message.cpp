/*
    Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>

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


#include "outgoing-message.h"

#include <KDebug>
#include <QSharedData>

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/Connection>

#include <TelepathyLoggerQt4/Entity>

using namespace KTp;

class OutgoingMessage::Private : public QSharedData {

  public:
    Private() {
        messageType = Tp::ChannelTextMessageTypeNormal;
    }
    QString text;
    Tp::ChannelTextMessageType messageType;
};

OutgoingMessage::OutgoingMessage(const OutgoingMessage &other):
    d(other.d)
{
}

OutgoingMessage& OutgoingMessage::operator=(const OutgoingMessage &other) {
    d = other.d;
    return *this;
}

OutgoingMessage::OutgoingMessage(const QString &messageText) :
    d(new Private)
{
    setText(messageText);
}

OutgoingMessage::~OutgoingMessage()
{
}

QString OutgoingMessage::text() const
{
    return d->text;
}

void OutgoingMessage::setText(const QString &text)
{
    d->text = text;
}

Tp::ChannelTextMessageType OutgoingMessage::type() const
{
    return d->messageType;
}

void OutgoingMessage::setType(Tp::ChannelTextMessageType type)
{
    d->messageType = type;
}
