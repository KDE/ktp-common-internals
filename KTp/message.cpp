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

using namespace KTp;

class Message::Private {

  public:
    Private()
    { }

    Private(const Private &other):
        sentTime(other.sentTime),
        token(other.token),
        messageType(other.messageType),
        properties(other.properties),
        mainPart(other.mainPart),
        parts(other.parts),
        scripts(other.scripts)
    { }

    QDateTime   sentTime;
    QString     token;
    Tp::ChannelTextMessageType messageType;
    QVariantMap properties;
    QString     mainPart;
    QStringList parts;
    QStringList scripts;
};

Message::Message(const Tp::Message &original) :
    d(new Private())
{
    d->sentTime = original.sent();
    d->token = original.messageToken();
    d->messageType = original.messageType();

    setMainMessagePart(original.text());
}

Message::Message(const Tpl::TextEventPtr &original) :
    d(new Private())
{
    d->sentTime = original->timestamp();
    d->token = original->messageToken();
    d->messageType = original->messageType();

    setMainMessagePart(original->message());
}

Message::Message(const Message& other):
    d(new Private(*(other.d)))
{
}

Message::~Message()
{
    delete d;
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

int Message::partsSize() const
{
    return d->parts.size();
}
