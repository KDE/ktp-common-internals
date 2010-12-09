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

#include "storage.h"

#include "ontologies/nco.h"
#include "ontologies/pimo.h"
#include "ontologies/telepathy.h"

#include <KDebug>

#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Thing>
#include <Nepomuk/Variant>

#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Query/NegationTerm>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/Result>

#include <TelepathyQt4/Constants>

ContactIdentifier::ContactIdentifier(const QString &accountId, const QString &contactId)
  : m_accountId(accountId),
    m_contactId(contactId)
{

}

ContactIdentifier::~ContactIdentifier()
{

}

const QString &ContactIdentifier::accountId() const
{
    return m_accountId;
}

const QString &ContactIdentifier::contactId() const
{
    return m_contactId;
}

bool ContactIdentifier::operator==(const ContactIdentifier& other) const
{
    return ((other.accountId() == accountId()) && (other.contactId() == contactId()));
}

bool ContactIdentifier::operator!=(const ContactIdentifier& other) const
{
    return !(*this == other);
}

ContactResources::ContactResources(const Nepomuk::PersonContact &personContact,
                                   const Nepomuk::IMAccount &imAccount)
  : m_personContact(personContact),
    m_imAccount(imAccount)
{

}

ContactResources::ContactResources()
{

}

ContactResources::~ContactResources()
{

}

const Nepomuk::PersonContact &ContactResources::personContact() const
{
    return m_personContact;
}

const Nepomuk::IMAccount &ContactResources::imAccount() const
{
    return m_imAccount;
}


Storage::Storage(QObject *parent)
: QObject(parent)
{
    // Create an instance of the Nepomuk Resource Manager, and connect to it's error signal.
    m_resourceManager = Nepomuk::ResourceManager::instance();

    connect(m_resourceManager,
            SIGNAL(error(QString, int)),
            SLOT(onNepomukError(QString, int)));

    // Here we get the "me" person contact.
    // FIXME: Port to new OSCAF standard for accessing "me" as soon as it
    // becomes available.
    Nepomuk::Thing me(QUrl::fromEncoded("nepomuk:/myself"));

    // FIXME: We should not create "me" if it doesn't exist once the above
    // fixme has been dealt with.
    if (!me.exists()) {
        // The PIMO:Person representing "me" does not exist, so we need to create it.
        kWarning() << "PIMO 'me' does not exist. Creating it.";
        me.addType(Nepomuk::Vocabulary::PIMO::Person());
    }

    // Loop through all the grounding instances of this person
    Q_FOREACH (Nepomuk::Resource resource, me.groundingOccurrences()) {
        // See if this grounding instance is of type nco:contact.
        if (resource.hasType(Nepomuk::Vocabulary::NCO::PersonContact())) {
            // FIXME: We are going to assume the first NCO::PersonContact is the
            // right one. Can we improve this?
            m_mePersonContact = resource;
            break;
        }
    }

    if (!m_mePersonContact.exists()) {
        kWarning() << "PersonContact 'me' does not exist. Creating it.";
        // FIXME: We shouldn't create this person contact, but for now we will
        // to ease development :) (see above FIXME's)
        m_mePersonContact = Nepomuk::PersonContact("nepomuk:/myself-person-contact");
        me.addGroundingOccurrence(m_mePersonContact);
    }
}

Storage::~Storage()
{
    // Don't delete the Nepomuk Resource manager. Nepomuk should take care of this itself.
}

void Storage::onNepomukError(const QString &uri, int errorCode)
{
    kWarning() << "A Nepomuk Error occurred:" << uri << errorCode;
}

void Storage::createAccount(const QString &path, const QString &id, const QString &protocol)
{
    kDebug() << "Creating a new Account";

    // First check if we already have this account.
    if (m_accounts.contains(path)) {
        kWarning() << "Account has already been created.";
        return;
    }

    // Query Nepomuk for IMAccount that the "me" PersonContact has with a specific path
    QList< Nepomuk::Query::Result > results;
    {
        using namespace Nepomuk::Query;

        // Match the account
        ComparisonTerm accountTerm(Nepomuk::Vocabulary::NCO::hasIMAccount(), ResourceTerm(m_mePersonContact));
        accountTerm.setInverted(true);

        // Match the ID
        ComparisonTerm pathTerm(Nepomuk::Vocabulary::Telepathy::accountIdentifier(),
                                LiteralTerm(path), Nepomuk::Query::ComparisonTerm::Equal);

        Query query(AndTerm(accountTerm, pathTerm,
                            ResourceTypeTerm(Nepomuk::Vocabulary::NCO::IMAccount())));

        bool queryResult = true;
        results = QueryServiceClient::syncQuery(query, &queryResult);

        if (!queryResult) {
            // TODO: Maybe an error notification here?
        }
    }
    // TODO: Maybe check if there is more than one result, and throw an error?
    kDebug() << results.size();

    // Iterate over all the IMAccounts found.
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::IMAccount foundImAccount(result.resource());
        kDebug() << this << ": Found IM Account: " << foundImAccount.uri();

        // See if the Account has the same Telepathy Account Identifier as the account this
        // TelepathyAccount instance has been created to look after.
        QStringList accountIdentifiers = foundImAccount.accountIdentifiers();

        if (accountIdentifiers.size() != 1) {
            kDebug() << "Account does not have 1 Telepathy Account Identifier. Oops. Ignoring."
                     << "Number of Identifiers: "
                     << accountIdentifiers.size();
            continue;
        }

        kDebug() << this << ": Found the corresponding IMAccount in Nepomuk.";
        // It matches.
        // Add the IMAccount to the hash.
        m_accounts.insert(path, foundImAccount);
        return;
    }

    kDebug() << "Could not find corresponding IMAccount in Nepomuk. Creating a new one.";

    Nepomuk::IMAccount imAccount;
    imAccount.addProperty(Nepomuk::Vocabulary::Telepathy::accountIdentifier(), path);
    imAccount.addProperty(Nepomuk::Vocabulary::NCO::imAccountType(), protocol);
    imAccount.addProperty(Nepomuk::Vocabulary::NCO::imID(), id);

    m_mePersonContact.addIMAccount(imAccount);

    // Add the account to the list.
    m_accounts.insert(path, imAccount);
}

void Storage::destroyAccount(const QString &path)
{
    // Check the account exists
    Q_ASSERT(m_accounts.contains(path));
    if (!m_accounts.contains(path)) {
        kWarning() << "Account not found.";
        return;
    }

    Nepomuk::IMAccount account = m_accounts.value(path);

    // The account object has been destroyed, which means we no longer know the presence of the
    // account, so it should be set to unknown.
    account.setProperty(Nepomuk::Vocabulary::NCO::imStatus(), QString::fromLatin1("unknown"));
    account.setProperty(Nepomuk::Vocabulary::Telepathy::statusType(), Tp::ConnectionPresenceTypeUnknown);
}

void Storage::setAccountNickname(const QString &path, const QString &nickname)
{
    // Check the account exists
    Q_ASSERT(m_accounts.contains(path));
    if (!m_accounts.contains(path)) {
        kWarning() << "Account not found.";
        return;
    }

    Nepomuk::IMAccount account = m_accounts.value(path);

    // Update the nickname property of the account.
    account.setProperty(Nepomuk::Vocabulary::NCO::imNickname(), nickname);
}

void Storage::setAccountCurrentPresence(const QString &path, const Tp::SimplePresence &presence)
{
    // Check the account exists
    Q_ASSERT(m_accounts.contains(path));
    if (!m_accounts.contains(path)) {
        kWarning() << "Account not found.";
        return;
    }

    Nepomuk::IMAccount account = m_accounts.value(path);

    // Update the Presence properties.
    account.setProperty(Nepomuk::Vocabulary::NCO::imStatus(), presence.status);
    account.setProperty(Nepomuk::Vocabulary::NCO::imStatusMessage(), presence.statusMessage);
    account.setProperty(Nepomuk::Vocabulary::Telepathy::statusType(), presence.type);
}

void Storage::createContact(const QString &path, const QString &id)
{
    // First, check that we don't already have a record for this contact.
    ContactIdentifier identifier(path, id);
    if (m_contacts.contains(identifier)) {
        kDebug() << "Contact record already exists. Return.";
        return;
    }

    kDebug() << "Don't yet have a record for this contact. Query Nepomuk.";

    // Assume that the Account this contact is related to exists.
    Q_ASSERT(m_accounts.contains(path));
    if (!m_accounts.contains(path)) {
        kWarning() << "Account not found.";
        return;
    }

    // Get the local account this contact is related to.
    Nepomuk::IMAccount account = m_accounts.value(path);

    // Query Nepomuk for all IM accounts that isBuddyOf the local account.
    QList< Nepomuk::Query::Result > results;
    {
        using namespace Nepomuk::Query;

        // Get the person contact owning this IMAccount
        ComparisonTerm pcterm(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                              ResourceTypeTerm(Nepomuk::Vocabulary::NCO::PersonContact()));
        pcterm.setVariableName("person");
        pcterm.setInverted(true);

        // Special case: if we're buddy of an account we do own, we want to create a new
        // resource for that.
        // This avoids race conditions and a lot of bad things.
        ComparisonTerm accountTerm(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                                   ResourceTerm(m_mePersonContact));
        accountTerm.setInverted(true);

        // And the ID has to match
        ComparisonTerm idTerm(Nepomuk::Vocabulary::NCO::imID(),
                              LiteralTerm(id), Nepomuk::Query::ComparisonTerm::Equal);

        // And the account has to be accessedBy this account.
        ComparisonTerm accessedByTerm(Nepomuk::Vocabulary::NCO::isAccessedBy(),
                                      ResourceTerm(account), Nepomuk::Query::ComparisonTerm::Equal);

        Query query(AndTerm(pcterm, idTerm, NegationTerm::negateTerm(accountTerm), accessedByTerm,
                            ResourceTypeTerm(Nepomuk::Vocabulary::NCO::IMAccount())));

        bool queryResult = true;
        results = QueryServiceClient::syncQuery(query, &queryResult);

        if (!queryResult) {
            // TODO: Maybe an error notification here?
        }
    }
    // TODO: Maybe check if there is more than one result, and throw an error?

    // Iterate over all the IMAccounts found.
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::IMAccount foundImAccount(result.resource());
        Nepomuk::IMAccount foundPersonContact(result.additionalBinding("person").toUrl());

        // Check that the IM account only has one ID.
        QStringList accountIDs = foundImAccount.imIDs();

        if (accountIDs.size() != 1) {
            kDebug() << "Account does not have 1 ID. Oops. Ignoring."
                        << "Number of Identifiers: "
                           << accountIDs.size();
                           continue;
        }
        // It matches, so cache the contact record.
        m_contacts.insert(identifier, ContactResources(foundPersonContact, foundImAccount));
        kDebug() << "Contact found in Nepomuk. Caching and Returning.";
        return;
    }

    // Contact not found. Need to create it.
    kDebug() << "Contact not found in Nepomuk. Creating it.";

    Nepomuk::PersonContact newPersonContact;
    Nepomuk::IMAccount newImAccount;

    newImAccount.setImStatus("unknown");
    newImAccount.setImIDs(QStringList() << id);
    newImAccount.setStatusTypes(QList<long long int>() << Tp::ConnectionPresenceTypeUnknown);
    newImAccount.setImAccountTypes(QStringList() << account.imAccountTypes().first());

    newImAccount.addIsAccessedBy(account);

    newPersonContact.addIMAccount(newImAccount);

    // Add it to the Contacts list.
    m_contacts.insert(identifier, ContactResources(newPersonContact, newImAccount));
}

void Storage::destroyContact(const QString &path, const QString &id)
{
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    Q_ASSERT(m_contacts.contains(identifier));
    if (!m_contacts.contains(identifier)) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = m_contacts.value(identifier);

    Nepomuk::IMAccount imAccount = resources.imAccount();
    Nepomuk::PersonContact personContact = resources.personContact();

    // The contact object has been destroyed, so we should set it's presence to unknown.
    imAccount.setProperty(Nepomuk::Vocabulary::NCO::imStatus(), QString::fromLatin1("unknown"));
    imAccount.setProperty(Nepomuk::Vocabulary::Telepathy::statusType(), Tp::ConnectionPresenceTypeUnknown);
}

void Storage::setContactAlias(const QString &path, const QString &id, const QString &alias)
{
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    Q_ASSERT(m_contacts.contains(identifier));
    if (!m_contacts.contains(identifier)) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = m_contacts.value(identifier);

    Nepomuk::IMAccount imAccount = resources.imAccount();
    Nepomuk::PersonContact personContact = resources.personContact();

    // Set the Contact Alias.
    imAccount.setImNicknames(QStringList() << alias);
}

void Storage::setContactPresence(const QString &path,
                                 const QString &id,
                                 const Tp::SimplePresence &presence)
{
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    Q_ASSERT(m_contacts.contains(identifier));
    if (!m_contacts.contains(identifier)) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = m_contacts.value(identifier);

    Nepomuk::IMAccount imAccount = resources.imAccount();
    Nepomuk::PersonContact personContact = resources.personContact();

    // Set the contact presence.
    imAccount.setImStatus(presence.status);
    imAccount.setStatusTypes(QList<long long int>() << presence.type);
    imAccount.setImStatusMessages(QStringList() << presence.statusMessage);
}

void Storage::addContactToGroup(const QString &path, const QString &id, const QString &group)
{
    // TODO: Implement me!
}

void Storage::removeContactFromGroup(const QString &path, const QString &id, const QString &group)
{
    // TODO: Implement me!
}

void Storage::setContactBlockStatus(const QString &path, const QString &id, bool blocked)
{
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    Q_ASSERT(m_contacts.contains(identifier));
    if (!m_contacts.contains(identifier)) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = m_contacts.value(identifier);

    Nepomuk::IMAccount imAccount = resources.imAccount();
    Nepomuk::PersonContact personContact = resources.personContact();

    // Set the blocked status.
    imAccount.setIsBlockeds(QList<bool>() << blocked);
}

void Storage::setContactPublishState(const QString &path,
                                     const QString &id,
                                     const Tp::Contact::PresenceState &state)
{
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    Q_ASSERT(m_contacts.contains(identifier));
    if (!m_contacts.contains(identifier)) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = m_contacts.value(identifier);

    Nepomuk::IMAccount imAccount = resources.imAccount();
    Nepomuk::PersonContact personContact = resources.personContact();

    // Get the local related account.
    Q_ASSERT(m_accounts.contains(path));
    if (!m_accounts.contains(path)) {
        kWarning() << "Account not found.";
        return;
    }

    Nepomuk::IMAccount localAccount = m_accounts.value(path);

    // Change the relationships based on the PresenceState value.
    if (state == Tp::Contact::PresenceStateYes) {
        // Add publishes to us.
        imAccount.addPublishesPresenceTo(localAccount);

        // Remove requested from us.
        QList<Nepomuk::IMAccount> rfs = localAccount.requestedPresenceSubscriptionTos();
        rfs.removeAll(imAccount);
        localAccount.setRequestedPresenceSubscriptionTos(rfs);

    } else if (state == Tp::Contact::PresenceStateAsk) {
        // We request subscription to them
        localAccount.addRequestedPresenceSubscriptionTo(imAccount);

        // Remove us from their publish list.
        QList<Nepomuk::IMAccount> pps = imAccount.publishesPresenceTos();
        pps.removeAll(localAccount);
        imAccount.setPublishesPresenceTos(pps);

    } else if (state == Tp::Contact::PresenceStateNo) {
        // Remove us from the requested-to-them list
        QList<Nepomuk::IMAccount> rfs = localAccount.requestedPresenceSubscriptionTos();
        rfs.removeAll(imAccount);
        localAccount.setRequestedPresenceSubscriptionTos(rfs);

        // Remove us from their publish list
        QList<Nepomuk::IMAccount> pps = imAccount.publishesPresenceTos();
        pps.removeAll(localAccount);
        imAccount.setPublishesPresenceTos(pps);

    } else {
        kWarning() << "Invalid Tp::Contact::PresenceState received.";
        Q_ASSERT(false);
    }
}

void Storage::setContactSubscriptionState(const QString &path,
                                          const QString &id,
                                          const Tp::Contact::PresenceState &state)
{
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    Q_ASSERT(m_contacts.contains(identifier));
    if (!m_contacts.contains(identifier)) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = m_contacts.value(identifier);

    Nepomuk::IMAccount imAccount = resources.imAccount();
    Nepomuk::PersonContact personContact = resources.personContact();

    // Get the local related account.
    Q_ASSERT(m_accounts.contains(path));
    if (!m_accounts.contains(path)) {
        kWarning() << "Account not found.";
        return;
    }

    Nepomuk::IMAccount localAccount = m_accounts.value(path);

    // Change the relationships based on the PresenceState value.
    if (state == Tp::Contact::PresenceStateYes) {
        // Add we publishes to them.
        localAccount.addPublishesPresenceTo(imAccount);

        // Remove requested from them.
        QList<Nepomuk::IMAccount> rfs = imAccount.requestedPresenceSubscriptionTos();
        rfs.removeAll(localAccount);
        imAccount.setRequestedPresenceSubscriptionTos(rfs);

    } else if (state ==  Tp::Contact::PresenceStateAsk) {
        // They request subscription to us
        imAccount.addRequestedPresenceSubscriptionTo(localAccount);

        // Remove them from our publish list.
        QList<Nepomuk::IMAccount> pps = localAccount.publishesPresenceTos();
        pps.removeAll(imAccount);
        localAccount.setPublishesPresenceTos(pps);

    } else if (state == Tp::Contact::PresenceStateNo) {
        // Remove them from the requested-to-us list
        QList<Nepomuk::IMAccount> rfs = imAccount.requestedPresenceSubscriptionTos();
        rfs.removeAll(localAccount);
        imAccount.setRequestedPresenceSubscriptionTos(rfs);

        // Remove them from our publish list
        QList<Nepomuk::IMAccount> pps = localAccount.publishesPresenceTos();
        pps.removeAll(imAccount);
        localAccount.setPublishesPresenceTos(pps);

    } else {
        kWarning() << "Invalid Tp::Contact::PresenceState received.";
        Q_ASSERT(false);
    }
}


int qHash(ContactIdentifier c)
{
    // FIXME: This is a shit way of doing it.
    QString temp = c.accountId();
    temp.append(QLatin1String("#--__--#"));
    temp.append(c.contactId());
    return qHash(temp);
}


#include "storage.moc"

