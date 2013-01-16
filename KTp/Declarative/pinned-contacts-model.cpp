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
#include <TelepathyQt/Contact>
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

#include <KTp/presence.h>

struct Pin {
    Tp::ContactPtr contact;
    Tp::AccountPtr account;
};

class PinnedContactsModelPrivate {
public:
    QVector<Pin> m_pins;
    Tp::AccountManagerPtr accountManager;
    ConversationsModel *convesations;

    QStringList pinsToString() const {
        QStringList ret;
        Q_FOREACH(const Pin &p, m_pins) {
            ret += p.account->uniqueIdentifier();
            ret += p.contact->id();
        }
        return ret;
    }
};

static Tp::AccountPtr findAccountByUniqueId(Tp::AccountManagerPtr manager, const QString &id)
{
    QList<Tp::AccountPtr> accounts = manager->allAccounts();
    Q_FOREACH(const Tp::AccountPtr account, accounts) {
        if (account->uniqueIdentifier() == id) {
            return account;
        }
    }
    Q_ASSERT(false && "account not found");
    return Tp::AccountPtr();
}

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
    Q_ASSERT(pins.size() % 2 == 0 && "the state should be a pair number of account id and contact name");
    if(!d->accountManager->isReady()) {
        Tp::PendingReady *r = d->accountManager->becomeReady();
        r->setProperty("newState", pins);
        connect(r, SIGNAL(finished(Tp::PendingOperation*)), SLOT(initializeState(Tp::PendingOperation*)));
    } else {
        kDebug() << "loading pinned...." << pins;
        for (int i = 0; i < pins.count(); i += 2) {
            Tp::AccountPtr account = findAccountByUniqueId(d->accountManager, pins[i]);
            if (account->connection()) {
                Tp::PendingContacts *pending = account->connection()->contactManager()->contactsForIdentifiers(QStringList(pins[i+1]));
                pending->setProperty("account", qVariantFromValue<Tp::AccountPtr>(account));
                connect(pending, SIGNAL(finished(Tp::PendingOperation*)), SLOT(pinPendingContacts(Tp::PendingOperation*)));
            }
        }
    }
}

void PinnedContactsModel::pinPendingContacts(Tp::PendingOperation* j)
{
    if (j->isValid()) {
        Tp::PendingContacts *job = qobject_cast<Tp::PendingContacts*>(j);
        Tp::AccountPtr account = job->property("account").value<Tp::AccountPtr>();
        Tp::ContactPtr contact = job->contacts().first();
        setPinning(account, contact, true);
    } else
        kDebug() << "error:" << j->errorName() << j->errorMessage();
}

QModelIndex PinnedContactsModel::indexForContact(Tp::AccountPtr account, Tp::ContactPtr contact) const
{
    int i = 0;
    Q_FOREACH(const Pin &p, d->m_pins) {
        if (p.account->uniqueIdentifier() == account->uniqueIdentifier() && p.contact->id() == contact->id()) {
            break;
        }
        i++;
    }
    if (i < d->m_pins.count()) {
        return index(i);
    } else {
        return QModelIndex();
    }
}

void PinnedContactsModel::setPinning(const Tp::AccountPtr &account, const Tp::ContactPtr &contact, bool newState)
{
    QModelIndex idx = indexForContact(account, contact);
    bool found = idx.isValid();
    if (newState && !found) {
        Pin p;
        p.account = account;
        p.contact = contact;
        appendContact(p);
    } else if (!newState && found) {
        removeRow(idx.row());
    }
}

QVariant PinnedContactsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        const Pin &p = d->m_pins[index.row()];
        switch(role) {
            case Qt::DisplayRole:
                return p.contact->alias();
            case PresenceIconRole:
                return KTp::Presence(p.contact->presence()).icon();
            case AvailabilityRole:
                return p.contact->presence().type()!=Tp::ConnectionPresenceTypeOffline
                    && p.contact->presence().type()!=Tp::ConnectionPresenceTypeError
                    && p.contact->presence().type()!=Tp::ConnectionPresenceTypeUnset
                    && p.contact->presence().type()!=Tp::ConnectionPresenceTypeUnknown;
            case ContactRole:
                return QVariant::fromValue<Tp::ContactPtr>(p.contact);
            case AccountRole:
                return QVariant::fromValue<Tp::AccountPtr>(p.account);
            case AlreadyChattingRole: {
                bool found = false;
                for (int i = 0; !found && i < d->convesations->rowCount(); i++) {
                    QModelIndex idx = d->convesations->index(i, 0);
                    Tp::ContactPtr contact = idx.data(ConversationsModel::ConversationRole).value<Conversation*>()->target()->contact();
                    found |= contact->id() == p.contact->id();
                }
                return found;
            }
            case Qt::DecorationRole: {
                KIcon icon;
                if (p.contact) {
                    icon = KIcon(p.contact->avatarData().fileName);
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

bool PinnedContactsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || (row + count) > d->m_pins.count()) {
        return false;
    }
    beginRemoveRows(parent, row, row + count - 1);
    d->m_pins.remove(row, count);
    endRemoveRows();
    return true;
}

void PinnedContactsModel::appendContact(const Pin &p)
{
    int s = d->m_pins.size();
    beginInsertRows(QModelIndex(), s, s);
    d->m_pins += p;
    endInsertRows();

    connect(p.contact.data(),
            SIGNAL(avatarDataChanged(Tp::AvatarData)),
            SLOT(contactDataChanged()));
    connect(p.contact.data(),
            SIGNAL(aliasChanged(QString)),
            SLOT(contactDataChanged()));
    connect(p.contact.data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SLOT(contactDataChanged()));
}

void PinnedContactsModel::contactDataChanged()
{
    Tp::Contact *c = qobject_cast<Tp::Contact*>(sender());
    int i = 0;
    Q_FOREACH(const Pin &p, d->m_pins) {
        if (p.contact == c) {
            QModelIndex idx = index(i);
            Q_EMIT dataChanged(idx, idx);
            return;
        }
        i++;
    }
}

void PinnedContactsModel::setConversationsModel(ConversationsModel *model)
{
    beginResetModel();
    d->convesations = model;
    connect(d->convesations, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)), SLOT(conversationsStateChanged(QModelIndex, int, int)));
    connect(d->convesations, SIGNAL(rowsInserted(QModelIndex, int, int)), SLOT(conversationsStateChanged(QModelIndex, int, int)));
    endResetModel();
}

void PinnedContactsModel::conversationsStateChanged(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; i++) {
        QModelIndex idx = d->convesations->index(i, 0, parent);
        Tp::ContactPtr contact = idx.data(ConversationsModel::ConversationRole).value<Conversation*>()->target()->contact();
        Q_FOREACH(const Pin &p, d->m_pins) {
            if (p.contact->id() == contact->id())
                QMetaObject::invokeMethod(this, "dataChanged", Qt::QueuedConnection, Q_ARG(QModelIndex, idx), Q_ARG(QModelIndex, idx));
        }
    }
}

ConversationsModel* PinnedContactsModel::conversationsModel() const
{
    return d->convesations;
}

Tp::AccountManagerPtr PinnedContactsModel::accountManager() const
{
    return d->accountManager;
}

void PinnedContactsModel::setAccountManager(const Tp::AccountManagerPtr &accounts)
{
    accounts->becomeReady();
    d->accountManager = accounts;
}

void PinnedContactsModel::initializeState(Tp::PendingOperation *op)
{
    setState(op->property("newState").toStringList());
}
