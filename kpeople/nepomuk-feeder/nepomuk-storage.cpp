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

#include "nepomuk-storage.h"

#include "ontologies/contactgroup.h"
#include "ontologies/nco.h"
#include "ontologies/pimo.h"
#include "ontologies/telepathy.h"
#include "ontologies/imcapability.h"
#include "ontologies/personcontact.h"
#include "ontologies/dataobject.h"

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

#include <QtCore/QSharedData>

#include <TelepathyQt4/Constants>
#include <TelepathyQt4/AvatarData>

class ContactIdentifier::Data : public QSharedData {
public:
    Data(const QString &a, const QString &c)
      : accountId(a),
        contactId(c)
    { }

    Data(const Data &other)
      : QSharedData(other),
        accountId(other.accountId),
        contactId(other.contactId)
    { }

    ~Data()
    { }

    QString accountId;
    QString contactId;
};

ContactIdentifier::ContactIdentifier(const QString &accountId, const QString &contactId)
  : d(new Data(accountId, contactId))
{

}

ContactIdentifier::ContactIdentifier(const ContactIdentifier &other)
  : d(other.d)
{

}

ContactIdentifier::~ContactIdentifier()
{

}

const QString &ContactIdentifier::accountId() const
{
    return d->accountId;
}

const QString &ContactIdentifier::contactId() const
{
    return d->contactId;
}

bool ContactIdentifier::operator==(const ContactIdentifier& other) const
{
    return ((other.accountId() == accountId()) && (other.contactId() == contactId()));
}

bool ContactIdentifier::operator!=(const ContactIdentifier& other) const
{
    return !(*this == other);
}


class ContactResources::Data : public QSharedData {
public:
    Data(const Nepomuk::PersonContact &p, const Nepomuk::IMAccount &i)
      : personContact(p),
        imAccount(i)
    { }

    Data()
    { }

    Data(const Data &other)
      : QSharedData(other),
        personContact(other.personContact),
        imAccount(other.imAccount)
    { }

    ~Data()
    { }

    Nepomuk::PersonContact personContact;
    Nepomuk::IMAccount imAccount;
};

ContactResources::ContactResources(const Nepomuk::PersonContact &personContact,
                                   const Nepomuk::IMAccount &imAccount)
  : d(new Data(personContact, imAccount))
{

}

ContactResources::ContactResources()
  : d(new Data())
{

}

ContactResources::ContactResources(const ContactResources &other)
  : d(other.d)
{

}

ContactResources::~ContactResources()
{

}

const Nepomuk::PersonContact &ContactResources::personContact() const
{
    return d->personContact;
}

const Nepomuk::IMAccount &ContactResources::imAccount() const
{
    return d->imAccount;
}


NepomukStorage::NepomukStorage(QObject *parent)
: AbstractStorage(parent)
{
    kDebug();

    // *********************************************************************************************
    // Nepomuk error handling

    // Create an instance of the Nepomuk Resource Manager, and connect to it's error signal.
    m_resourceManager = Nepomuk::ResourceManager::instance();

    connect(m_resourceManager,
            SIGNAL(error(QString,int)),
            SLOT(onNepomukError(QString,int)));

    // *********************************************************************************************
    // Get the ME PIMO:Person and NCO:PersonContact (creating them if necessary)

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

    // *********************************************************************************************
    // Load all the relevant accounts and contacts that are already in Nepomuk.

    // We load everything now, because this is much more efficient than re-running a query for each
    // and every account and contact individually when we get to that one.

    // Query Nepomuk for all of the ME PersonContact's IMAccounts.
    QList<Nepomuk::Query::Result> results;
    {
        using namespace Nepomuk::Query;

        ComparisonTerm accountTerm(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                                   ResourceTerm(m_mePersonContact));
        accountTerm.setInverted(true);

        Query query(AndTerm(accountTerm, ResourceTypeTerm(Nepomuk::Vocabulary::NCO::IMAccount())));

        bool queryResult = true;
        results = QueryServiceClient::syncQuery(query, &queryResult);

        if (!queryResult) {
            kWarning() << "Query failed.";
        }
    }

    kDebug() << results.size();

    // Iterate over all the IMAccounts found.
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::IMAccount foundImAccount(result.resource());
        kDebug() << this << ": Found IM Account: " << foundImAccount.uri();

        // If no Telepathy identifier, then the account is ignored.
        if (foundImAccount.accountIdentifier().isEmpty()) {
            kDebug() << "Account does not have a Telepathy Account Identifier. Oops. Ignoring.";
            continue;
        }

        kDebug() << "Found a Telepathy account in Nepomuk, ID:"
                 << foundImAccount.accountIdentifier();

        // If it does have a telepathy identifier, then it is added to the cache.
        m_accounts.insert(foundImAccount.accountIdentifier(), foundImAccount);
    }

    // Query Nepomuk for all know Contacts.
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

        ComparisonTerm accessedByTerm(Nepomuk::Vocabulary::NCO::isAccessedBy(),
                                      ResourceTypeTerm(Nepomuk::Vocabulary::NCO::IMAccount()));
        accessedByTerm.setVariableName("accessedBy");

        Query query(AndTerm(pcterm, NegationTerm::negateTerm(accountTerm), accessedByTerm,
                            ResourceTypeTerm(Nepomuk::Vocabulary::NCO::IMAccount())));

        bool queryResult = true;
        results = QueryServiceClient::syncQuery(query, &queryResult);

        if (!queryResult) {
            kWarning() << "Query failed.";
        }
    }

    // Iterate over all the IMAccounts found.
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::IMAccount foundImAccount(result.resource());
        Nepomuk::IMAccount foundPersonContact(result.additionalBinding("person").toUrl());
        Nepomuk::IMAccount foundImAccountAccessedBy(result.additionalBinding("accessedBy").toUrl());

        // Check that the IM account only has one ID.
        QStringList accountIDs = foundImAccount.imIDs();

        if (accountIDs.size() != 1) {
            kDebug() << "Account does not have 1 ID. Oops. Ignoring."
            << "Number of Identifiers: "
            << accountIDs.size();
            continue;
        }

        // Cache the contact
        m_contacts.insert(ContactIdentifier(foundImAccountAccessedBy.accountIdentifier(),
                                            foundImAccount.imIDs().first()),
                          ContactResources(foundPersonContact, foundImAccount));
        kDebug() << "Contact found in Nepomuk. Caching.";
    }
}

NepomukStorage::~NepomukStorage()
{
    // Don't delete the Nepomuk Resource manager. Nepomuk should take care of this itself.
}

void NepomukStorage::onNepomukError(const QString &uri, int errorCode)
{
    kWarning() << "A Nepomuk Error occurred:" << uri << errorCode;
}

void NepomukStorage::createAccount(const QString &path, const QString &id, const QString &protocol)
{
    kDebug() << "Creating a new Account";

    // First check if we already have this account.
    if (m_accounts.contains(path)) {
        kWarning() << "Account has already been created.";
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

void NepomukStorage::destroyAccount(const QString &path)
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

void NepomukStorage::setAccountNickname(const QString &path, const QString &nickname)
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

void NepomukStorage::setAccountCurrentPresence(const QString &path, const Tp::SimplePresence &presence)
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

void NepomukStorage::createContact(const QString &path, const QString &id)
{
    // First, check that we don't already have a record for this contact.
    ContactIdentifier identifier(path, id);
    if (m_contacts.contains(identifier)) {
        kDebug() << "Contact record already exists. Return.";
        return;
    }

    // Contact not found. Need to create it.
    kDebug() << "Contact not found in Nepomuk. Creating it.";

    Nepomuk::IMAccount account(m_accounts.value(path));
    Q_ASSERT(m_accounts.keys().contains(path));
    if (!m_accounts.keys().contains(path)) {
        kWarning() << "Corresponding account not cached.";
        return;
    }

    Nepomuk::PersonContact newPersonContact;
    Nepomuk::IMAccount newImAccount;
    Nepomuk::Thing newPimoPerson;
    newPimoPerson.addType(Nepomuk::Vocabulary::PIMO::Person());

    newImAccount.setImStatus("unknown");
    newImAccount.setImIDs(QStringList() << id);
    newImAccount.setStatusType(Tp::ConnectionPresenceTypeUnknown);
    newImAccount.setImAccountType(account.imAccountType());

    newImAccount.addIsAccessedBy(account);

    newPersonContact.addIMAccount(newImAccount);

    newPimoPerson.addGroundingOccurrence(newPersonContact);

    // Add it to the Contacts list.
    m_contacts.insert(identifier, ContactResources(newPersonContact, newImAccount));
}

void NepomukStorage::destroyContact(const QString &path, const QString &id)
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

    // The contact object has been destroyed, so we should set it's presence to unknown.
    imAccount.setProperty(Nepomuk::Vocabulary::NCO::imStatus(), QString::fromLatin1("unknown"));
    imAccount.setProperty(Nepomuk::Vocabulary::Telepathy::statusType(), Tp::ConnectionPresenceTypeUnknown);
}

void NepomukStorage::setContactAlias(const QString &path, const QString &id, const QString &alias)
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

    if (!imAccount.imNicknames().contains(alias)) {
        // Set the Contact Alias.
        imAccount.setImNicknames(QStringList() << alias);
    }
}

void NepomukStorage::setContactPresence(const QString &path,
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

    // Set the contact presence.
    if (imAccount.imStatus() != presence.status) {
        imAccount.setImStatus(presence.status);
    }

    if (imAccount.statusType() != presence.type) {
        imAccount.setStatusType(presence.type);
    }

    if (imAccount.imStatusMessage() != presence.statusMessage) {
        imAccount.setImStatusMessage(presence.statusMessage);
    }
}

void NepomukStorage::setContactGroups(const QString &path,
                                      const QString &id,
                                      const QStringList &groups)
{
    kDebug() << "Set Groups Starting";
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    Q_ASSERT(m_contacts.contains(identifier));
    if (!m_contacts.contains(identifier)) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = m_contacts.value(identifier);
    Nepomuk::PersonContact personContact = resources.personContact();

    // Set the contact groups.
    // First remove any groups we are no longer a member of.
    QList<Nepomuk::ContactGroup> newGroups = personContact.belongsToGroups();

    foreach (const Nepomuk::ContactGroup &group, personContact.belongsToGroups()) {
        if (!groups.contains(group.contactGroupName())) {
            newGroups.removeAll(group);
        }
    }

    // Now add any groups we are newly a member of.
    bool found;
    foreach (const QString &groupName, groups) {
        found = false;
        foreach (const Nepomuk::ContactGroup &cGroup, newGroups) {
            if (cGroup.contactGroupName() == groupName) {
                found = true;
                break;
            }
        }

        if (!found) {
            // Not already in that group. Check the group exists.
            // FIXME: Once we have a "ContactList" resource for Telepathy Contacts, we should only
            //        get the groups associated with that.
            Nepomuk::ContactGroup groupResource;
            foreach (const Nepomuk::ContactGroup &g, Nepomuk::ContactGroup::allContactGroups()) {
                if (g.contactGroupName() == groupName) {
                    groupResource = g;
                    break;
                }
            }

            // If the group doesn't already exist, create it.
            if (groupResource.resourceUri().isEmpty()) {
                // FIXME: Once we have a "ContactList" resource for Telepathy Contacts, we should
                //        create this group as a child of that resource.
                groupResource.setContactGroupName(groupName);
            }

            newGroups.append(groupResource);
        }
    }

    // Update the groups property with the new list
    personContact.setBelongsToGroups(newGroups);
    kDebug() << "Set Groups Ending";
}

void NepomukStorage::setContactBlockStatus(const QString &path, const QString &id, bool blocked)
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

    // Set the blocked status.
    if (imAccount.isBlocked() != blocked) {
        imAccount.setIsBlocked(blocked);
    }
}

void NepomukStorage::setContactPublishState(const QString &path,
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

void NepomukStorage::setContactSubscriptionState(const QString &path,
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

void NepomukStorage::setContactCapabilities(const QString &path,
                                           const QString &id,
                                           const Tp::ContactCapabilities &capabilities)
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

    // For each supported (by the ontology) capability, check it and save the correct value
    QSet<Nepomuk::IMCapability> caps(imAccount.iMCapabilitys().toSet());

    if (capabilities.textChats()) {
        caps.insert(Nepomuk::Vocabulary::NCO::imCapabilityText());
    } else {
        caps.remove(Nepomuk::Vocabulary::NCO::imCapabilityText());
    }

    if (capabilities.streamedMediaAudioCalls()) {
        caps.insert(Nepomuk::Vocabulary::NCO::imCapabilityAudio());
    } else {
        caps.remove(Nepomuk::Vocabulary::NCO::imCapabilityAudio());
    }

    if (capabilities.streamedMediaVideoCalls()) {
        caps.insert(Nepomuk::Vocabulary::NCO::imCapabilityVideo());
    } else {
        caps.remove(Nepomuk::Vocabulary::NCO::imCapabilityVideo());
    }

    // FIXME: Add other caps to the ontologies so that we can add them here.

    // Save the new list of caps.
    imAccount.setIMCapabilitys(caps.toList());
}

void NepomukStorage::setContactAvatar(const QString &path,
                                      const QString &id,
                                      const Tp::AvatarData &avatar)
{
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    Q_ASSERT(m_contacts.contains(identifier));
    if (!m_contacts.contains(identifier)) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = m_contacts.value(identifier);

    Nepomuk::PersonContact personContact = resources.personContact();
    Nepomuk::IMAccount imAccount = resources.imAccount();

    // Get the resource uri of the avatar.
    Nepomuk::DataObject avatarPhoto = imAccount.avatar();

    // Get all the photos of the person contact
    QList<Nepomuk::DataObject> photos = personContact.photos();

    // Remove the one we are removing.
    photos.removeAll(avatarPhoto);

    // Update the photos
    personContact.setPhotos(photos);

    if (avatar.fileName.isEmpty()) {
        kDebug() << "No avatar set.";
        // Remove the avatar property
        imAccount.removeProperty(Nepomuk::Vocabulary::Telepathy::avatar());

    } else {
        // Add the new avatar to Nepomuk, both as the avatar and as a photo.
        Nepomuk::Resource newAvatarPhoto(avatar.fileName);
        personContact.addPhoto(newAvatarPhoto);
        imAccount.setAvatar(newAvatarPhoto);
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


#include "nepomuk-storage.moc"

