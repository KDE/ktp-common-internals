/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2009-2011 Collabora Ltd. <info@collabora.co.uk>
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

#ifndef NEPOMUK_TELEPATHY_SERVICE_NEPOMUK_STORAGE_H
#define NEPOMUK_TELEPATHY_SERVICE_NEPOMUK_STORAGE_H

#include "abstract-storage.h"

#include "ontologies/imaccount.h"
#include "ontologies/personcontact.h"

#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Types>

namespace Nepomuk {
    class ResourceManager;
}

class ContactIdentifier {
public:
    ContactIdentifier(const QString &accountId, const QString &contactId);
    ContactIdentifier(const ContactIdentifier &other);
    ~ContactIdentifier();

    const QString &accountId() const;
    const QString &contactId() const;

    bool operator==(const ContactIdentifier &other) const;
    bool operator!=(const ContactIdentifier &other) const;

private:
    class Data;
    QSharedDataPointer<Data> d;
};

int qHash(ContactIdentifier c);

class ContactResources {
public:
    ContactResources(const Nepomuk::PersonContact &personContact,
                     const Nepomuk::IMAccount &imAccount);
    ContactResources(const ContactResources &other);
    ContactResources();
    ~ContactResources();

    const Nepomuk::PersonContact &personContact() const;
    const Nepomuk::IMAccount &imAccount() const;

private:
    class Data;
    QSharedDataPointer<Data> d;
};

/**
 * All interaction with the Nepomuk database takes place in this class.
 */
class NepomukStorage : public AbstractStorage
{
    Q_OBJECT

public:
    explicit NepomukStorage(QObject *parent = 0);
    virtual ~NepomukStorage();

public Q_SLOTS:
    virtual void createAccount(const QString &path, const QString &id, const QString &protocol);
    virtual void destroyAccount(const QString &path);
    virtual void setAccountNickname(const QString &path, const QString &nickname);
    virtual void setAccountCurrentPresence(const QString &path, const Tp::SimplePresence &presence);

    virtual void createContact(const QString &path, const QString &id);
    virtual void destroyContact(const QString &path, const QString &id);
    virtual void setContactAlias(const QString &path, const QString &id, const QString &alias);
    virtual void setContactPresence(const QString &path, const QString &id, const Tp::SimplePresence &presence);
    virtual void setContactGroups(const QString &path, const QString &id, const QStringList &groups);
    virtual void setContactBlockStatus(const QString &path, const QString &id, bool blocked);
    virtual void setContactPublishState(const QString &path, const QString &id, const Tp::Contact::PresenceState &state);
    virtual void setContactSubscriptionState(const QString &path, const QString &id, const Tp::Contact::PresenceState &state);
    virtual void setContactCapabilities(const QString &path, const QString &id, const Tp::ContactCapabilities &capabilities);

private Q_SLOTS:
    void onNepomukError(const QString &uri, int errorCode);

private:
    Q_DISABLE_COPY(NepomukStorage);

    friend class TestBackdoors;

    Nepomuk::ResourceManager *m_resourceManager;
    Nepomuk::PersonContact m_mePersonContact;

    QHash<QString, Nepomuk::IMAccount> m_accounts;
    QHash<ContactIdentifier, ContactResources> m_contacts;
};


#endif // Header guard

