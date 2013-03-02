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

    setProperty("sender", context.account()->nickname());
    setProperty("sender-avatar", context.account()->avatar().avatarData);
}

Message::Message(const Tp::ReceivedMessage &original, const KTp::MessageContext &context) :
    d(new Private)
{
    Q_UNUSED(context)
    d->sentTime = original.sent();
    d->token = original.messageToken();
    d->messageType = original.messageType();
    d->isHistory = original.isScrollback();
    d->direction = KTp::Message::RemoteToLocal;

    setMainMessagePart(original.text());

    if (!original.sender().isNull()) {
        setProperty("sender", original.sender()->alias());
        setProperty("sender-avatar", original.sender()->avatarData().fileName);
    } else {
        setProperty("sender", original.senderNickname());
    }
}

Message::Message(const Tpl::TextEventPtr &original, const KTp::MessageContext &context) :
    d(new Private)
{
    d->sentTime = original->timestamp();
    d->token = original->messageToken();
    d->messageType = original->messageType();
    d->isHistory = true;

    if (original->sender()->identifier() == context.account()->normalizedName()) {
        d->direction = KTp::Message::LocalToRemote;
    } else {
        d->direction = KTp::Message::RemoteToLocal;
    }

    setMainMessagePart(original->message());

    setProperty("sender", original->sender()->alias());
    setProperty("sender-avatar", original->sender()->avatarToken());
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

QString Message::sender() const
{
    return property("sender").toString();
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
