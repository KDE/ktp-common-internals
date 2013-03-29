/*
    Copyright (C) 2012 Aleix Pol <aleixpol@kde.org>

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

#include "pinned-contacts-model.h"
#include "conversations-model.h"
#include "conversation.h"

#include <TelepathyQt/Types>
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/Presence>
#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

#include <KIcon>
#include <KGlobal>
#include <KComponentData>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KDebug>

#include "KTp/presence.h"
#include "KTp/contact.h"
#include "KTp/persistent-contact.h"

class PinnedContactsModelPrivate {
public:
    PinnedContactsModelPrivate() {
        conversations = 0;
    }

    QList<KTp::PersistentContactPtr> m_pins;
    Tp::AccountManagerPtr accountManager;
    ConversationsModel *conversations;

    QStringList pinsToString() const {
        QStringList ret;
        Q_FOREACH(const KTp::PersistentContactPtr &p, m_pins) {
            ret += p->accountId();
            ret += p->contactId();
        }
        return ret;
    }
};

PinnedContactsModel::PinnedContactsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new PinnedContactsModelPrivate)
{
    QHash<int, QByteArray> roles = roleNames();
    roles[PresenceIconRole] = "presenceIcon";
    roles[AvailabilityRole] = "available";
    roles[ContactRole] = "contact";
    roles[AccountRole] = "account";
    roles[AlreadyChattingRole] = "alreadyChatting";
    setRoleNames(roles);

    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(countChanged()));
}

PinnedContactsModel::~PinnedContactsModel()
{
    delete d;
}

QStringList PinnedContactsModel::state() const
{
    return d->pinsToString();
}

void PinnedContactsModel::setState(const QStringList &pins)
{
    for (int i = 0; i < pins.count(); i += 2) {
        appendContactPin(KTp::PersistentContact::create(pins[0], pins[1]));
    }
}

QModelIndex PinnedContactsModel::indexForContact(const KTp::ContactPtr &contact) const
{
    for (int i=0; i<d->m_pins.size();i++) {
        if (d->m_pins[i]->contact() == contact) {
            return index(i);
        }
    }
    return QModelIndex();
}

void PinnedContactsModel::setPinning(const Tp::AccountPtr &account, const KTp::ContactPtr &contact, bool newState)
{
    QModelIndex idx = indexForContact(contact);
    bool found = idx.isValid();
    if (newState && !found) {
        KTp::PersistentContactPtr p = KTp::PersistentContact::create(account->uniqueIdentifier(), contact->id());
        appendContactPin(p);
    } else if (!newState && found) {
        removeContactPin(d->m_pins[idx.row()]);
    }
}

QVariant PinnedContactsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        KTp::PersistentContactPtr p = d->m_pins[index.row()];
        switch(role) {
        case Qt::DisplayRole:
            if (p->contact()) {
                return p->contact()->alias();
            }
            break;
        case PresenceIconRole:
            if (p->contact()) {
                return KTp::Presence(p->contact()->presence()).icon();
            } else {
                return KTp::Presence(Tp::Presence::offline()).icon();
            }
            break;
        case AvailabilityRole:
            if (!p->contact()) {
                return false;
            }
            else {
                return p->contact()->presence().type()!=Tp::ConnectionPresenceTypeOffline
                        && p->contact()->presence().type()!=Tp::ConnectionPresenceTypeError
                        && p->contact()->presence().type()!=Tp::ConnectionPresenceTypeUnset
                        && p->contact()->presence().type()!=Tp::ConnectionPresenceTypeUnknown;
            }
        case ContactRole:
            return QVariant::fromValue<KTp::ContactPtr>(p->contact());
        case AccountRole:
            return QVariant::fromValue<Tp::AccountPtr>(p->account());
        case AlreadyChattingRole: {
            if (!p->contact()) {
                return false;
            }
            bool found = false;
            for (int i = 0; !found && i < d->conversations->rowCount(); i++) {
                QModelIndex idx = d->conversations->index(i, 0);
                KTp::ContactPtr contact = idx.data(ConversationsModel::ConversationRole).value<Conversation*>()->target()->contact();
                found |= contact->id() == p->contact()->id();
            }
            return found;
        }
        case Qt::DecorationRole: {
            KIcon icon;
            if (p->contact()) {
                QString file = p->contact()->avatarData().fileName;
                if (!file.isEmpty()) {
                    icon = KIcon(file);
                }
            }
            if (icon.isNull()) {
                icon = KIcon(QLatin1String("im-user"));
            }
            return icon;
        }
        }
    }
    return QVariant();
}

int PinnedContactsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->m_pins.count();
}

void PinnedContactsModel::removeContactPin(const KTp::PersistentContactPtr &pin)
{
    int row = d->m_pins.indexOf(pin);
    beginRemoveRows(QModelIndex(), row, row);
    d->m_pins.removeAt(row);
    endRemoveRows();
}

void PinnedContactsModel::appendContactPin(const KTp::PersistentContactPtr &pin)
{
    int s = d->m_pins.size();
    beginInsertRows(QModelIndex(), s, s);
    d->m_pins += pin;
    endInsertRows();

    if (d->accountManager && d->accountManager->isReady()) {
        pin->setAccountManager(d->accountManager);
    }

    if (pin->contact()) {
        contactChanged(pin->contact());
    }
    connect(pin.data(), SIGNAL(contactChanged(KTp::ContactPtr)), SLOT(contactChanged(KTp::ContactPtr)));
}

void PinnedContactsModel::contactChanged(const KTp::ContactPtr &contact)
{
    if (contact) {
        connect(contact.data(),
                SIGNAL(avatarDataChanged(Tp::AvatarData)),
                SLOT(contactDataChanged()));
        connect(contact.data(),
                SIGNAL(aliasChanged(QString)),
                SLOT(contactDataChanged()));
        connect(contact.data(),
                SIGNAL(presenceChanged(Tp::Presence)),
                SLOT(contactDataChanged()));
    }

    QModelIndex index = indexForContact(contact);
    Q_EMIT dataChanged(index, index);
}

void PinnedContactsModel::contactDataChanged()
{
    KTp::Contact *c = qobject_cast<KTp::Contact*>(sender());
    QModelIndex index = indexForContact(KTp::ContactPtr(c));
    Q_EMIT dataChanged(index, index);
}

void PinnedContactsModel::setConversationsModel(ConversationsModel *model)
{
    beginResetModel();
    d->conversations = model;
    connect(d->conversations, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)), SLOT(conversationsStateChanged(QModelIndex, int, int)));
    connect(d->conversations, SIGNAL(rowsInserted(QModelIndex, int, int)), SLOT(conversationsStateChanged(QModelIndex, int, int)));
    endResetModel();
}

void PinnedContactsModel::conversationsStateChanged(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; i++) {
        QModelIndex idx = d->conversations->index(i, 0, parent);
        Conversation* conv = idx.data(ConversationsModel::ConversationRole).value<Conversation*>();
        KTp::ContactPtr contact = conv->target()->contact();
        Q_FOREACH(const KTp::PersistentContactPtr &p, d->m_pins) {
            if (p->contact() == contact) {
                QModelIndex contactIdx = indexForContact(p->contact());
                //We need to delay the dataChanged until the next event loop, when endRowsRemoved has been called
                QMetaObject::invokeMethod(this, "dataChanged", Qt::QueuedConnection, Q_ARG(QModelIndex, contactIdx), Q_ARG(QModelIndex, contactIdx));
            }
        }
    }
}

ConversationsModel* PinnedContactsModel::conversationsModel() const
{
    return d->conversations;
}

Tp::AccountManagerPtr PinnedContactsModel::accountManager() const
{
    return d->accountManager;
}

void PinnedContactsModel::setAccountManager(const Tp::AccountManagerPtr &accounts)
{
    d->accountManager = accounts;

    connect(d->accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)), SLOT(onAccountManagerReady()));
}

void PinnedContactsModel::onAccountManagerReady()
{
    Q_FOREACH(const KTp::PersistentContactPtr &p, d->m_pins) {
        p->setAccountManager(d->accountManager);
    }
}
