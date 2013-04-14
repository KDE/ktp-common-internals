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


#include "message.h"

#include <KDebug>
#include <QSharedData>

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/Connection>

#include <TelepathyLoggerQt4/Entity>

using namespace KTp;

class Message::Private : public QSharedData {

  public:
    Private() :
        isHistory(false)
    {}

    QDateTime   sentTime;
    QString     token;
    Tp::ChannelTextMessageType messageType;
    QVariantMap properties;
    QString     mainPart;
    QStringList parts;
    QStringList scripts;
    bool isHistory;
    MessageDirection direction;

    //if we have a valid sender pointer store that, otherwise store alias
    KTp::ContactPtr sender;
    QString senderAlias;
    QString senderId;
};

Message& Message::operator=(const Message &other) {
    d = other.d;
    return *this;
}

Message::Message(const Tp::Message &original, const KTp::MessageContext &context) :
    d(new Private)
{
    Q_UNUSED(context)
    d->sentTime = original.sent();
    d->token = original.messageToken();
    d->messageType = original.messageType();
    d->isHistory = false;
    d->direction = KTp::Message::LocalToRemote;

    setMainMessagePart(original.text());

    if (context.account()->connection()) {
        d->sender = KTp::ContactPtr::qObjectCast(context.account()->connection()->selfContact());
    } else {
        d->senderAlias = context.account()->nickname();
        d->senderId = context.account()->uniqueIdentifier();
    }
}

Message::Message(const Tp::ReceivedMessage &original, const KTp::MessageContext &context) :
    d(new Private)
{
    Q_UNUSED(context)

    d->sentTime = original.sent();
    if (d->sentTime.isNull()) {
        d->sentTime = original.received();
    }

    d->token = original.messageToken();
    d->messageType = original.messageType();
    d->isHistory = original.isScrollback();
    d->direction = KTp::Message::RemoteToLocal;

    setMainMessagePart(original.text());

    if (!original.sender().isNull()) {
        d->sender = KTp::ContactPtr::qObjectCast(original.sender());
    } else {
        d->senderAlias = original.senderNickname();
    }
}

Message::Message(const Tpl::TextEventPtr &original, const KTp::MessageContext &context) :
    d(new Private)
{
    d->sentTime = original->timestamp();
    d->token = original->messageToken();
    d->messageType = original->messageType();
    d->isHistory = true;

    d->senderAlias = original->sender()->alias();
    d->senderId = original->sender()->identifier();

    if (original->sender()->identifier() == context.account()->normalizedName()) {
        d->direction = KTp::Message::LocalToRemote;
    } else {
        d->direction = KTp::Message::RemoteToLocal;
    }

    if (context.channel()) {
        Q_FOREACH (const Tp::ContactPtr &contact, context.channel()->groupContacts()) {
            if (contact->id() == original->sender()->identifier()) {
                d->sender = KTp::ContactPtr::qObjectCast(contact);
            }
        }
    }

    setMainMessagePart(original->message());
}

Message::Message(const QString &messageText, const MessageContext &context) :
    d(new Private)
{
    d->sentTime = QDateTime::currentDateTime();
    d->messageType = Tp::ChannelTextMessageTypeNormal;
    d->direction = LocalToRemote;
    d->isHistory = false;

    setMainMessagePart(messageText);
}

Message::Message(const Message& other):
    d(other.d)
{
}

Message::~Message()
{
}

QString Message::mainMessagePart() const
{
    return d->mainPart;
}

void Message::setMainMessagePart(const QString& message)
{
    d->mainPart = message;
}

void Message::appendMessagePart(const QString& part)
{
    d->parts << part;
}

void Message::appendScript(const QString& script)
{
    // Append the script only if it is not already appended to avoid multiple
    // execution of the scripts.
    if (!d->scripts.contains(script)) {
        d->scripts << script;
    }
}

QString Message::finalizedMessage() const
{
    QString msg = d->mainPart + QLatin1String("\n") +
        d->parts.join(QLatin1String("\n"));

//     kDebug() << msg;
    return msg;
}

QString Message::finalizedScript() const
{
    if (d->scripts.empty()) {
        return QString();
    }

    QString finalScript = d->scripts.join(QLatin1String(""));

    if (!finalScript.isEmpty()) {
        finalScript.append(QLatin1String("false;"));
    }

//    kDebug() << finalScript;
    return finalScript;
}

QVariant Message::property(const char *name) const
{
    return d->properties[QLatin1String(name)];
}

void Message::setProperty(const char *name, const QVariant& value)
{
    d->properties[QLatin1String(name)] = value;
}

QDateTime Message::time() const
{
    return d->sentTime;
}

QString Message::token() const
{
    return d->token;
}

Tp::ChannelTextMessageType Message::type() const
{
    return d->messageType;
}

QString Message::senderAlias() const
{
    if (d->sender) {
        return d->sender->alias();
    }
    return d->senderAlias;
}

QString Message::senderId() const
{
    if (d->sender) {
        return d->sender->id();
    }
    return d->senderId;
}

KTp::ContactPtr Message::sender() const
{
    return d->sender;
}

int Message::partsSize() const
{
    return d->parts.size();
}

bool Message::isHistory() const
{
    return d->isHistory;
}

KTp::Message::MessageDirection Message::direction() const
{
    return d->direction;
}
