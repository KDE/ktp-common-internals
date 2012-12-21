/*
* Copyright (C) 2012 David Edmundson <kde@davidedmundson.co.uk>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "contacts-list-model.h"
#include "contact.h"

#include <TelepathyQt/Account>
#include <TelepathyQt/Contact>
#include <TelepathyQt/ContactCapabilities>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ContactManager>

#include <KTp/Models/contacts-model.h>
#include <KTp/presence.h>

class KTp::ContactsListModel::Private
{
public:
    QList<Tp::ContactPtr> contacts;
    KTp::GlobalContactManager *contactManager;
};


KTp::ContactsListModel::ContactsListModel(QObject *parent) :
    QAbstractListModel(parent),
    d(new KTp::ContactsListModel::Private())
{
    d->contactManager = 0;
}

KTp::ContactsListModel::~ContactsListModel()
{
    delete d;
}

void KTp::ContactsListModel::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    d->contactManager = new KTp::GlobalContactManager(accountManager, this);
    onContactsChanged(d->contactManager->allKnownContacts(), Tp::Contacts());
    connect(d->contactManager, SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts)), SLOT(onContactsChanged(Tp::Contacts,Tp::Contacts)));
}

int KTp::ContactsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED (parent);
    return d->contacts.size();
}

QVariant KTp::ContactsListModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    if (row >=0 && row < d->contacts.size()) {
        const KTp::ContactPtr contact = KTp::ContactPtr::qObjectCast(d->contacts[row]);

        switch (role) {
        case ContactsModel::IdRole:
            return contact->id();
        case ContactsModel::TypeRole:
            return ContactsModel::ContactRowType;
        case ContactsModel::ContactRole:
            return QVariant::fromValue(d->contacts[row]); //Tp::ContactPtr and NOT KTp::ContactPtr
        case ContactsModel::AccountRole:
            return QVariant::fromValue(d->contactManager->accountForContact(contact));
        case Qt::DisplayRole:
            return contact->alias();
        case ContactsModel::AliasRole:
            return contact->alias();
        case ContactsModel::PresenceRole:
            return QVariant::fromValue(contact->presence());
        case ContactsModel::PresenceIconRole:
            return QIcon(contact->presence().icon());
        case ContactsModel::PresenceStatusRole:
            return contact->presence().status();
        case ContactsModel::PresenceTypeRole:
            return contact->presence().type();
        case ContactsModel::PresenceMessageRole:
            return contact->presence().statusMessage();
        case ContactsModel::SubscriptionStateRole:
            return contact->subscriptionState();
        case ContactsModel::PublishStateRole:
            return contact->publishState();
        case ContactsModel::BlockedRole:
            return contact->isBlocked();
        case ContactsModel::GroupsRole:
            return contact->groups();
        case ContactsModel::AvatarRole:
            return contact->avatarData().fileName;
        case Qt::DecorationRole:
            return QImage(contact->avatarData().fileName);
        case ContactsModel::TextChatCapabilityRole:
            return contact->capabilities().textChats();
        case ContactsModel::ClientTypesRole:
            return contact->clientTypes();
        default:
            break;
        }

    }
    return QVariant();
}

void KTp::ContactsListModel::onContactsChanged(const Tp::Contacts &added, const Tp::Contacts &removed)
{
    //add contacts.

    Q_FOREACH(const Tp::ContactPtr &contact_uncasted, added) {
        KTp::ContactPtr contact = KTp::ContactPtr::qObjectCast(contact_uncasted);

        connect(contact.data(),
                    SIGNAL(aliasChanged(QString)),
                    SLOT(onChanged()));
            connect(contact.data(),
                    SIGNAL(avatarTokenChanged(QString)),
                    SLOT(onChanged()));
            connect(contact.data(),
                    SIGNAL(avatarDataChanged(Tp::AvatarData)),
                    SLOT(onChanged()));
            connect(contact.data(),
                    SIGNAL(presenceChanged(Tp::Presence)),
                    SLOT(onChanged()));
            connect(contact->manager()->connection()->selfContact().data(),
                    SIGNAL(capabilitiesChanged(Tp::ContactCapabilities)),
                    SLOT(onChanged()));
            connect(contact.data(),
                    SIGNAL(capabilitiesChanged(Tp::ContactCapabilities)),
                    SLOT(onChanged()));
            connect(contact.data(),
                    SIGNAL(locationUpdated(Tp::LocationInfo)),
                    SLOT(onChanged()));
            connect(contact.data(),
                    SIGNAL(infoFieldsChanged(Tp::Contact::InfoFields)),
                    SLOT(onChanged()));
            connect(contact.data(),
                    SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
                    SLOT(onChanged()));
            connect(contact.data(),
                    SIGNAL(publishStateChanged(Tp::Contact::PresenceState,QString)),
                    SLOT(onChanged()));
            connect(contact.data(),
                    SIGNAL(blockStatusChanged(bool)),
                    SLOT(onChanged()));
            connect(contact.data(),
                    SIGNAL(clientTypesChanged(QStringList)),
                    SLOT(onChanged()));

            connect(contact.data(), SIGNAL(invalidated()), SLOT(onConnectionDropped()));
    }

    if (added.size() > 0) {
        beginInsertRows(QModelIndex(), d->contacts.size(), d->contacts.size() + added.size() -1);
        d->contacts.append(added.toList());
        endInsertRows();
    }

    //remove contacts
    Q_FOREACH(const Tp::ContactPtr &contact, removed) {
        int row = d->contacts.indexOf(contact);
        if (row >= 0) { //if contact found in list
            beginRemoveRows(QModelIndex(), row, row);
            d->contacts.removeOne(contact);
            endInsertRows();
        }
    }
}

void KTp::ContactsListModel::onChanged()
{
    KTp::ContactPtr contact(qobject_cast<KTp::Contact*>(sender()));
    int row = d->contacts.indexOf(contact);
    if (row > 0) {
        QModelIndex index = createIndex(row, 0);
        dataChanged(index, index);
    }
}

void KTp::ContactsListModel::onConnectionDropped()
{
    KTp::ContactPtr contact(qobject_cast<KTp::Contact*>(sender()));
    onContactsChanged(Tp::Contacts(), Tp::Contacts() << contact);
}

