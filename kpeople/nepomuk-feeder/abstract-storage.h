/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2010-2011 Collabora Ltd. <info@collabora.co.uk>
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

#ifndef NEPOMUK_TELEPATHY_SERVICE_ABSTRACT_STORAGE_H
#define NEPOMUK_TELEPATHY_SERVICE_ABSTRACT_STORAGE_H

#include <QtCore/QObject>
#include <QtCore/QString>

#include <TelepathyQt/Contact>
#include <TelepathyQt/ContactCapabilities>
#include <TelepathyQt/Types>

/**
 * Abstract interface class for storage implementations. This allows fake storage classes to be used
 * when, e.g. writing unit tests, to remove the need for a sandboxed Nepomuk set up.
 *
 * To implement a storage class, subclass this class, implementing all the pure-abstract methods.
 */
class AbstractStorage : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit AbstractStorage(QObject *parent = 0);

    /**
     * Destructor
     */
    virtual ~AbstractStorage();

public Q_SLOTS:
    /**
     * Invoked when the account manager is readied, this method allows the storage class to alter
     * accounts that are no longer in the account manager as well as batch-adding any new accounts
     * that are not yet in the store.
     *
     * \param paths the object paths (unique identifiers) of the complete list of Telepathy Accounts.
     */
    virtual void cleanupAccounts(const QList<QString> &paths) = 0;

    /**
     * Invoked when a Telepathy Account is constructed.
     *
     * \param path the object path (unique identifier) of the account
     * \param id the id (e.g. me@example.com) of the account
     * \param protocol the protocol string (e.g. msn or xmpp) of the account
     */
    virtual void createAccount(const QString &path, const QString &id, const QString &protocol) = 0;

    /**
     * Invoked to update the nickname of the Telepathy Account.
     *
     * \param path the object path (unique identifier) of the account
     * \param nickname the account's nickname
     */
    virtual void setAccountNickname(const QString &path, const QString &nickname) = 0;

    /**
     * Invoked when the contact list of a Telepathy Account becomes available, this method allows
     * the storage class to alter contacts that are no longer part of the server-side contact list
     * as well as batch-adding any contacts to the store that have been added to the server-side
     * contact list since the last run.
     *
     * \param path the object path (unique identifier) of the account
     * \param ids the ids (e.g. me@example.com) of the complete server-side contact list of the account.
     */
    virtual void cleanupAccountContacts(const QString &path, const Tp::Contacts &contacts) = 0;

    /**
     * Invoked when the Tp account is removed from AccountManager
     *
     * \param path the object path of the account
     */
    virtual void onAccountRemoved(const QString &path) = 0;


    /**
     * Invoked when a Telepathy Contact is constructed.
     *
     * \param path the object path (unique identifier) of the account
     * \param id the id (e.g. me@example.com) that in conjunction with the \p path uniquely identifies the contact.
     */
    virtual void createContact(const QString &path, const Tp::ContactPtr &contact) = 0;

    /**
     * Invoked when a Telepathy Contact is removed.
     *
     * \param path the object path (unique identifier) of the account
     * \param id the id (e.g. me@example.com) that in conjunction with the \p path uniquely identifies the contact.
     */
    virtual void removeContact(const QString &path, const Tp::ContactPtr &contact) = 0;


    /**
     * Invoked to update the alias of the Telepathy Contact.
     *
     * \param path the object path (unique identifier) of the account
     * \param id the id (e.g. me@example.com) that in conjunction with the \p path uniquely identifies the contact.
     * \param alias the contact's alias.
     */
    virtual void setContactAlias(const QString &path, const QString &id, const QString &alias) = 0;

    /**
     * Invoked to update the groups to which the Telepahy Contact belongs.
     *
     * \param path the object path (unique identifier) of the account
     * \param id the id (e.g. me@example.com) that in conjunction with the \p path uniquely identifies the contact.
     * \param groups the complete list of groups to which the contact belongs.
     */
    virtual void setContactGroups(const QString &path, const QString &id, const QStringList &groups) = 0;

    /**
     * Invoked to update the avatar of the Telepathy Contact.
     *
     * \param path the object path (unique identifier) of the account
     * \param id the id (e.g. me@example.com) that in conjunction with the \p path uniquely identifies the contact.
     * \param avatar the contact's avatar.
     */
    virtual void setContactAvatar(const QString &path, const QString &id, const Tp::AvatarData &avatar) = 0;

Q_SIGNALS:
    /**
     * Signal emitted to indicate whether the storage instance was successfully initialised. The
     * use of this signal allows for the storage to use asyncrhonous operations to complete its
     * initialisation.
     *
     * \param success indicates whether the storage instance was initialised successfully.
     */
    void initialised(bool success);

private:
    Q_DISABLE_COPY(AbstractStorage);
};


#endif // Header guard

