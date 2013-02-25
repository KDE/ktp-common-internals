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


#ifndef CONVERSATION_TARGET_H
#define CONVERSATION_TARGET_H

#include <QObject>
#include <QIcon>

#include "KTp/types.h"

class ConversationTarget : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QIcon avatar READ avatar NOTIFY avatarChanged);
    Q_PROPERTY(QString nick READ nick NOTIFY nickChanged);
    Q_PROPERTY(QIcon presenceIcon READ presenceIcon NOTIFY presenceIconChanged);
    Q_PROPERTY(QString presenceIconName READ presenceIconName NOTIFY presenceIconNameChanged)
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(KTp::ContactPtr contact READ contact CONSTANT)
    Q_PROPERTY(Tp::AccountPtr account READ account CONSTANT)

  public:
    explicit ConversationTarget(const Tp::AccountPtr &account, const KTp::ContactPtr &contact, QObject *parent = 0);
    virtual ~ConversationTarget();

    QIcon   avatar() const;
    QString id() const;
    QString nick() const;
    QIcon   presenceIcon() const;
    QString presenceIconName() const;

    KTp::ContactPtr contact() const;
    Tp::AccountPtr account() const;

  Q_SIGNALS:
    void avatarChanged(QIcon avatar);
    void nickChanged(QString nick);
    void presenceIconChanged(QIcon icon);
    void presenceIconNameChanged(QString icon);

  private Q_SLOTS:
    void onAvatarDataChanged();
    void onPresenceChanged();

  private:
    void setupContactSignals(KTp::ContactPtr contact);
    void updateAvatar();

    class ConversationTargetPrivate;
    ConversationTargetPrivate *d;
};

Q_DECLARE_METATYPE(ConversationTarget*)

#endif // CONVERSATION_TARGET_H
