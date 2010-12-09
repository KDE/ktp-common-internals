/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
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

#ifndef NEPOMUK_TELEPATHY_SERVICE_STORAGE_H
#define NEPOMUK_TELEPATHY_SERVICE_STORAGE_H

#include "ontologies/imaccount.h"
#include "ontologies/personcontact.h"

#include <QtCore/QObject>
#include <QtCore/QString>

#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Types>

namespace Nepomuk {
    class ResourceManager;
}

class ContactIdentifier {
public:
    ContactIdentifier(const QString &accountId, const QString &contactId);
    ~ContactIdentifier();

    const QString &accountId() const;
    const QString &contactId() const;

    bool operator==(const ContactIdentifier &other) const;
    bool operator!=(const ContactIdentifier &other) const;

private:
    QString m_accountId;
    QString m_contactId;
};

class ContactResources {
public:
    ContactResources(const Nepomuk::PersonContact &personContact,
                     const Nepomuk::IMAccount &imAccount);
    ContactResources();
    ~ContactResources();

    const Nepomuk::PersonContact &personContact() const;
    const Nepomuk::IMAccount &imAccount() const;

private:
    Nepomuk::PersonContact m_personContact;
    Nepomuk::IMAccount m_imAccount;
};

/**
 * All interaction with the Nepomuk database takes place in this class.
 */
class Storage : public QObject
{
    Q_OBJECT

public:
    explicit Storage(QObject *parent = 0);
    ~Storage();

public Q_SLOTS:
    void createAccount(const QString &path, const QString &id, const QString &protocol);
    void destroyAccount(const QString &path);
    void setAccountNickname(const QString &path, const QString &nickname);
    void setAccountCurrentPresence(const QString &path, const Tp::SimplePresence &presence);

    void createContact(const QString &path, const QString &id);
    void destroyContact(const QString &path, const QString &id);
    void setContactAlias(const QString &path, const QString &id, const QString &alias);
    void setContactPresence(const QString &path, const QString &id, const Tp::SimplePresence &presence);
    void addContactToGroup(const QString &path, const QString &id, const QString &group);
    void removeContactFromGroup(const QString &path, const QString &id, const QString &group);
    void setContactBlockStatus(const QString &path, const QString &id, bool blocked);
    void setContactPublishState(const QString &path, const QString &id, const Tp::Contact::PresenceState &state);
    void setContactSubscriptionState(const QString &path, const QString &id, const Tp::Contact::PresenceState &state);

private Q_SLOTS:
    void onNepomukError(const QString &uri, int errorCode);

private:
    Q_DISABLE_COPY(Storage);

    Nepomuk::ResourceManager *m_resourceManager;
    Nepomuk::PersonContact m_mePersonContact;

    QHash<QString, Nepomuk::IMAccount> m_accounts;
    QHash<ContactIdentifier, ContactResources> m_contacts;
};

int qHash(ContactIdentifier c);


#endif // Header guard

