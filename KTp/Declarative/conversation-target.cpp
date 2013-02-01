/*
    Copyright (C) 2011  Lasath Fernando <kde@lasath.org>

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

#include "conversation-target.h"
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/Contact>
#include <KDebug>
#include <KTp/presence.h>

class  ConversationTarget::ConversationTargetPrivate
{
  public:
    KTp::ContactPtr contact;
    KIcon avatar;
    Tp::AccountPtr account;
};

ConversationTarget::ConversationTarget(const Tp::AccountPtr &account, const KTp::ContactPtr &contact, QObject *parent) :
    QObject(parent),
    d(new ConversationTargetPrivate)
{
    kDebug();

    if (contact) {
        setupContactSignals(contact);
    }

    d->contact = contact;
    d->account = account;
    updateAvatar();
}

void ConversationTarget::setupContactSignals(KTp::ContactPtr contact)
{
    connect(contact.constData(), SIGNAL(aliasChanged(QString)), SIGNAL(nickChanged(QString)));
    connect(contact.constData(), SIGNAL(avatarDataChanged(Tp::AvatarData)), SLOT(onAvatarDataChanged()));
    connect(contact.constData(), SIGNAL(presenceChanged(Tp::Presence)), SLOT(onPresenceChanged()));
}

QIcon ConversationTarget::avatar() const
{
    if (d->contact) {
        return d->avatar;
    } else {
        return QIcon();
    }
}

QString ConversationTarget::id() const
{
    if (d->contact) {
        return d->contact->id();
    } else {
        return QString();
    }
}

QString ConversationTarget::nick() const
{
    if (d->contact) {
        return d->contact->alias();
    } else {
        return QString();
    }
}

QIcon ConversationTarget::presenceIcon() const
{
    if (d->contact) {
        return KTp::Presence(d->contact->presence()).icon();
    } else {
        return QIcon();
    }
}

QString ConversationTarget::presenceIconName() const
{
    if (d->contact) {
       return KTp::Presence(d->contact->presence()).iconName();
    } else {
       return QString();
    }
}

void ConversationTarget::onPresenceChanged()
{
    Q_EMIT presenceIconChanged(presenceIcon());
    Q_EMIT presenceIconNameChanged(presenceIconName());
}

void ConversationTarget::onAvatarDataChanged()
{
    updateAvatar();
    Q_EMIT avatarChanged(avatar());
}

void ConversationTarget::updateAvatar()
{
    QString path;
    if (d->contact) {
        path = d->contact->avatarData().fileName;
    }

    if (path.isEmpty()) {
        path = QLatin1String("im-user");
    }

    d->avatar = KIcon(path);
}

KTp::ContactPtr ConversationTarget::contact() const
{
    return d->contact;
}

Tp::AccountPtr ConversationTarget::account() const
{
    return d->account;
}

ConversationTarget::~ConversationTarget()
{
    delete d;
}
