/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2009-2011 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
 *
 * Copyright (C) 2011-2012 Vishesh Handa <handa.vish@gmail.com>
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

#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

#include <TelepathyQt/Contact>
#include <TelepathyQt/Types>

#include <nepomuk2/simpleresourcegraph.h>
#include <KJob>

namespace Nepomuk2 {
    class ResourceManager;
    namespace Query {
        class Result;
    }
}

class KJob;

class AccountResources {
public:
    AccountResources(const QUrl &account,
                     const QString &protocol);
    AccountResources(const AccountResources &other);
    AccountResources(const QUrl &url);
    AccountResources();
    ~AccountResources();

    const QUrl &account() const;
    const QString &protocol() const;

    bool operator==(const AccountResources &other) const;
    bool operator!=(const AccountResources &other) const;
    bool operator==(const QUrl &other) const;
    bool operator!=(const QUrl &other) const;

    bool isEmpty() const;
private:
    class Data;
    QSharedDataPointer<Data> d;
};

class ContactIdentifier {
public:
    ContactIdentifier(const QString &accountId, const QString &contactId);
    ContactIdentifier(const ContactIdentifier &other);
    ContactIdentifier();
    ~ContactIdentifier();

    const QString &accountId() const;
    const QString &contactId() const;

    bool operator==(const ContactIdentifier &other) const;
    bool operator!=(const ContactIdentifier &other) const;

private:
    class Data;
    QSharedDataPointer<Data> d;
};

uint qHash(const ContactIdentifier& c);

class ContactResources {
public:
    ContactResources(const QUrl &personContact,
                     const QUrl &imAccount);
    ContactResources(const ContactResources &other);
    ContactResources();
    ~ContactResources();

    const QUrl &personContact() const;
    const QUrl &imAccount() const;

    bool operator==(const ContactResources &other) const;
    bool operator!=(const ContactResources &other) const;

    bool isEmpty() const;
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
    virtual void cleanupAccounts(const QList<QString> &paths);
    virtual void createAccount(const QString &path, const QString &id, const QString &protocol);
    virtual void setAccountNickname(const QString &path, const QString &nickname);
    virtual void cleanupAccountContacts(const QString &path, const Tp::Contacts &contacts);
    virtual void onAccountRemoved(const QString &path);

    virtual void removeContact(const QString &path, const Tp::ContactPtr &contact);
    virtual void createContact(const QString &path, const Tp::ContactPtr &contact);
    virtual void setContactAlias(const QString &path, const QString &id, const QString &alias);
    virtual void setContactGroups(const QString &path, const QString &id, const QStringList &groups);
    virtual void setContactAvatar(const QString &path, const QString &id, const Tp::AvatarData &avatar);

Q_SIGNALS:
    void graphSaved();

private Q_SLOTS:
    void init();

    void onAccountsQueryFinishedListing();
    void onContactTimer();
    void onContactGraphJob(KJob *job);


private:
    void updateAlias(Nepomuk2::SimpleResource &contactResource, Nepomuk2::SimpleResource &imAccountResource, const QString &alias);
    void updateContactGroups(Nepomuk2::SimpleResource &contactResource, const QStringList &groups);
    void updateContactAvatar(Nepomuk2::SimpleResource &contactResource, Nepomuk2::SimpleResource &imAccountResource, const Tp::AvatarData &avatar);

    Q_DISABLE_COPY(NepomukStorage);

    friend class TestBackdoors;

    QUrl m_mePersonContact;

    QHash<QString, AccountResources> m_accounts;
    QHash<ContactIdentifier, ContactResources> m_contacts;

    Nepomuk2::SimpleResourceGraph m_graph;
    QTimer m_graphTimer;

    ContactResources findContact(const QString &path, const QString &id);
    AccountResources findAccount(const QString &path);

    void fireGraphTimer();

    QUrl findGroup(const QString& groupName);
    QHash<QString, QUrl> m_groupCache;
};


#endif // Header guard

