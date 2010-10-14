/*
 * This file is part of nepomuktelepathyservice
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
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

#include "telepathycontact.h"

#include "telepathyaccount.h"
#include "telepathyaccountmonitor.h"

#include <TelepathyQt4/ContactCapabilities>

// Ontology Vocabularies
#include "ontologies/nco.h"
#include "ontologies/pimo.h"
#include "ontologies/telepathy.h"

// Full Resource Classes
#include "ontologies/contactgroup.h"
#include "ontologies/dataobject.h"
#include "ontologies/imcapability.h"

#include <KDebug>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Thing>
#include <Nepomuk/Variant>

#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Query/NegationTerm>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/Result>

TelepathyContact::TelepathyContact(Tp::ContactPtr contact,
                                   Tp::ConnectionPtr connection,
                                   Nepomuk::IMAccount accountResource,
                                   TelepathyAccount *parent)
 : QObject(parent),
   m_parent(parent),
   m_contact(contact),
   m_connection(connection),
   m_accountResource(accountResource)
{
    kDebug() << "New TelepathyContact Created:"
             << m_contact.data()
             << m_connection.data()
             << m_parent;

    // We need to destroy ourself if the connection goes down.
    connect(m_connection.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(deleteLater()));

    // Connect to signals for all properties we want to keep synced.
    connect(m_contact.data(),
            SIGNAL(simplePresenceChanged(QString,uint,QString)),
            SLOT(onPresenceChanged(QString,uint,QString)));
    connect(m_contact.data(),
            SIGNAL(aliasChanged(QString)),
            SLOT(onAliasChanged(QString)));
    connect(m_contact.data(),
            SIGNAL(addedToGroup(QString)),
            SLOT(onAddedToGroup(QString)));
    connect(m_contact.data(),
            SIGNAL(removedFromGroup(QString)),
            SLOT(onRemovedFromGroup(QString)));
    connect(m_contact.data(),
            SIGNAL(capabilitiesChanged(Tp::ContactCapabilities*)),
            SLOT(onCapabilitiesChanged(Tp::ContactCapabilities*)));
    // Watch over his subscription, publication and block status
    connect(m_contact.data(),
            SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
            SLOT(onSubscriptionStateChanged(Tp::Contact::PresenceState)));
    connect(m_contact.data(),
            SIGNAL(publishStateChanged(Tp::Contact::PresenceState)),
            SLOT(onPublishStateChanged(Tp::Contact::PresenceState)));
    connect(m_contact.data(),
            SIGNAL(blockStatusChanged(bool)),
            SLOT(onBlockStatusChanged(bool)));
    // FIXME: Connect to any other signals of sync-worthy properties here.

    // Find the Nepomuk resource for this contact, or create a new one if necessary.
    doNepomukSetup();

    // Take the needed action for the current presence/subscription/block state
    onPublishStateChanged(m_contact->publishState());
    onSubscriptionStateChanged(m_contact->subscriptionState());
    onBlockStatusChanged(m_contact->isBlocked());
    if (contact->capabilities() != 0) {
        onCapabilitiesChanged(m_contact->capabilities());
    }
}

TelepathyContact::~TelepathyContact()
{
    kDebug();

    // If the IMAccount resource already exsits, set it to offline presence when we are destroyed.
    if (!m_contactIMAccountResource.uri().isEmpty()) {
        m_contactIMAccountResource.setProperty(Nepomuk::Vocabulary::NCO::imStatus(), "offline");
        m_contactIMAccountResource.setProperty(Nepomuk::Vocabulary::NCO::imStatusMessage(), "");
        m_contactIMAccountResource.setProperty(Nepomuk::Vocabulary::Telepathy::statusType(), 0);
    }

    // Signal this contact is destroyed so it can be removed from the Hash.
    Q_EMIT contactDestroyed(m_contact);
}

void TelepathyContact::doNepomukSetup()
{
    // Query Nepomuk for all IM accounts that isBuddyOf the accountResource
    QList< Nepomuk::Query::Result > results;
    {
        using namespace Nepomuk::Query;

        // Get the person contact owning this IMAccount
        ComparisonTerm pcterm(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                              ResourceTypeTerm(Nepomuk::Vocabulary::NCO::PersonContact()));
        pcterm.setVariableName("person");
        pcterm.setInverted(true);

        // Get a copy of the "me" PersonContact.
        Nepomuk::PersonContact mePersonContact = m_parent->monitor()->mePersonContact();
        // Special case: if we're buddy of an account we do own, we want to create a new resource for that.
        // This avoids race conditions and a lot of bad things.
        ComparisonTerm accountTerm(Nepomuk::Vocabulary::NCO::hasIMAccount(), ResourceTerm(mePersonContact));
        accountTerm.setInverted(true);

        // And the ID has to match
        ComparisonTerm idTerm(Nepomuk::Vocabulary::NCO::imID(),
                              LiteralTerm(m_contact->id()), Nepomuk::Query::ComparisonTerm::Equal);

        Query query(AndTerm(pcterm, idTerm, NegationTerm::negateTerm(accountTerm),
                            ResourceTypeTerm(Nepomuk::Vocabulary::NCO::IMAccount())));

        bool queryResult = true;
        results = QueryServiceClient::syncQuery(query, &queryResult);

        if (!queryResult) {
            // TODO: Maybe an error notification here?
        }
    }
    // TODO: Maybe check if there is more than one result, and throw an error?
    kDebug() << "Querying contact " << m_contact->id() << m_accountResource.accountIdentifiers().first()
             << ": found " << results.count();

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

        // It matches, so set our member variables to found resources and stop looping.
        m_contactIMAccountResource = foundImAccount;
        m_contactPersonContactResource = foundPersonContact;

        // Sync any properties that may have changed since last time we were online.
        if (m_contactIMAccountResource.property(Nepomuk::Vocabulary::NCO::imNickname())
            != m_contact->alias()) {
            onAliasChanged(m_contact->alias());
        }
        onPresenceChanged(m_contact->presenceStatus(),
                            m_contact->presenceType(),
                            m_contact->presenceMessage()); // We can always assume this one needs syncing.
        // Call onAddedToGroup for all the groups this contact is in. (it will ignore ones
        // where Nepomuk already knows the contact is in this group.
        foreach (const QString &group, m_contact->groups()) {
            onAddedToGroup(group);
        }
        // FIXME: What other properties do we need to sync?

        break;
    }

    // If the contactIMAccountResource is still empty, create a new IMAccount and PersonContact.
    if (m_contactIMAccountResource.uri().isEmpty()) {
        kDebug() << this << ": Create new IM Account and corresponding PersonContact";

        m_contactIMAccountResource.addProperty(Nepomuk::Vocabulary::NCO::imID(),
                                               m_contact->id());
        m_contactIMAccountResource.addProperty(Nepomuk::Vocabulary::NCO::imNickname(),
                                               m_contact->alias());
        m_contactIMAccountResource.addProperty(Nepomuk::Vocabulary::NCO::imStatus(),
                                               m_contact->presenceStatus());
        m_contactIMAccountResource.addProperty(Nepomuk::Vocabulary::NCO::imStatusMessage(),
                                               m_contact->presenceMessage());
        m_contactIMAccountResource.addProperty(Nepomuk::Vocabulary::Telepathy::statusType(),
                                               m_contact->presenceType());

        m_contactPersonContactResource.addProperty(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                                                   m_contactIMAccountResource);
        // FIXME: Store any other relevant Contact properties to Nepomuk.
        // Create a PIMO Person for the new contact.
        Nepomuk::Thing pimoPerson;
        pimoPerson.addType(Nepomuk::Vocabulary::PIMO::Person());
        pimoPerson.addGroundingOccurrence(m_contactPersonContactResource);
    }
}

void TelepathyContact::onAliasChanged(const QString& alias)
{
    // Only set properties if we have already got the contactIMAccountResource.
    if (!m_contactIMAccountResource.uri().isEmpty()) {
        m_contactIMAccountResource.setProperty(Nepomuk::Vocabulary::NCO::imNickname(),
                                               alias);
    }
}

void TelepathyContact::onPresenceChanged(const QString& status, uint type, const QString& message)
{
    // Only set properties if we have already got the contactIMAccountResource.
    if (!m_contactIMAccountResource.uri().isEmpty()) {
        m_contactIMAccountResource.setProperty(Nepomuk::Vocabulary::NCO::imStatus(),
                                               status);
        m_contactIMAccountResource.setProperty(Nepomuk::Vocabulary::NCO::imStatusMessage(),
                                               message);
        m_contactIMAccountResource.setProperty(Nepomuk::Vocabulary::Telepathy::statusType(),
                                               type);
    }
}

void TelepathyContact::onAddedToGroup(const QString &group)
{
    kDebug() << "On added to group " << group;
    // Only update the properties if we already have the contactIMAccountResource.
    if (!m_contactIMAccountResource.resourceUri().isEmpty()) {

        // Check if the contact is already in that group
        foreach(Nepomuk::ContactGroup g, m_contactPersonContactResource.belongsToGroups()) {
            if (g.contactGroupName() == group) {
                // Already in that group
                kDebug() << "Already in that group " << group;
                return;
            }
        }

        // Not already in that group. Check the group exists.
        // FIXME: Once we have a "ContactList" resource for Telepathy Contacts, we should only
        //        get the groups associated with that.
        Nepomuk::ContactGroup groupResource;
        foreach (Nepomuk::ContactGroup g, Nepomuk::ContactGroup::allContactGroups()) {
            if (g.contactGroupName() == group) {
                groupResource = g;
                break;
            }
        }

        // If the group doesn't already exist, create it.
        if (groupResource.resourceUri().isEmpty()) {
            kDebug() << "Resource URI is empty.";
            // FIXME: Once we have a "ContactList" resource for Telepathy Contacts, we should
            //        create this group as a child of that resource.
            groupResource.setContactGroupName(group);
        }

        // Add the contact to the group.
        kDebug() << "Added to " << group;
        m_contactPersonContactResource.addBelongsToGroup(groupResource);
    }
}

void TelepathyContact::onRemovedFromGroup(const QString &group)
{
    kDebug() << "On removed from group " << group;
    // Only update the properties if we already have the contactIMAccountResource.
    if (!m_contactIMAccountResource.resourceUri().isEmpty()) {

        // Loop through all the contact's groups and if we find one with the
        // right name, remove it from the list.
        QList<Nepomuk::ContactGroup> oldGroups = m_contactPersonContactResource.belongsToGroups();
        QList<Nepomuk::ContactGroup> newGroups = m_contactPersonContactResource.belongsToGroups();

        foreach (const Nepomuk::ContactGroup &g, oldGroups) {
            if (g.contactGroupName() == group) {
                newGroups.removeAll(g);
            }
        }

        kDebug() << "Setting groups to " << newGroups;

        m_contactPersonContactResource.setBelongsToGroups(newGroups);
    }
}

QString TelepathyContact::avatarToken() const
{
    // Only check the properties if we already have the contactPersonAccountResource.
    if (!m_contactPersonContactResource.resourceUri().isEmpty()) {
        // Return the stored token
        return m_contactPersonContactResource.property(Nepomuk::Vocabulary::Telepathy::avatarToken()).toString();
    }

    kDebug() << "Resource URI empty";
    return QString::null;
}

void TelepathyContact::setAvatar(const QString& token, const QByteArray& data)
{
    kDebug() << "Storing avatar for " << token << m_contact.data();
    // Only update the properties if we already have the contactIMAccountResource.
    if (!m_contactPersonContactResource.resourceUri().isEmpty()) {
        // Add token...
        m_contactPersonContactResource.setAvatarTokens(QStringList() << token);
        // .. and the data itself
        Nepomuk::InformationElement photo;
        photo.setPlainTextContents(QStringList() << data.toBase64());
        Nepomuk::DataObject dataObject(m_contactPersonContactResource);
        dataObject.addInterpretedAs(photo);
        m_contactPersonContactResource.setPhotos(QList<Nepomuk::DataObject>() << dataObject);
    }
}

void TelepathyContact::onBlockStatusChanged(bool blocked)
{
    // Only update the properties if we already have the contactIMAccountResource.
    if (!m_contactPersonContactResource.resourceUri().isEmpty()) {
        if (m_contactIMAccountResource.isBlockeds().isEmpty()) {
            // Add the property
            m_contactIMAccountResource.setProperty(Nepomuk::Vocabulary::Telepathy::isBlocked(),
                                                   blocked);
        }

        if (blocked != m_contactIMAccountResource.isBlockeds().first()) {
            m_contactIMAccountResource.setProperty(Nepomuk::Vocabulary::Telepathy::isBlocked(),
                                                   blocked);
        }
    }
}

void TelepathyContact::onPublishStateChanged(Tp::Contact::PresenceState state)
{
    Q_UNUSED(state)
    kDebug() << "Publication changed!";
    switch (state) {
        case Tp::Contact::PresenceStateYes:
            // We can now see the contact's presence

            // Only update the properties if we already have the contactIMAccountResource.
            if (!m_contactIMAccountResource.resourceUri().isEmpty()) {
                // If we're not yet isBuddyOf, let's become one.
                if (!m_contactIMAccountResource.publishesPresenceTos().contains(m_accountResource)) {
                    kDebug() << "We're not yet publishing presence to the account";
                    m_contactIMAccountResource.addPublishesPresenceTo(m_accountResource);
                }
            }
            kDebug() << "State yes, now it publishes to " << m_contactIMAccountResource.publishesPresenceTos();

            break;
        case Tp::Contact::PresenceStateNo:
            // We can no longer see his presence

            // Only update the properties if we already have the contactIMAccountResource.
            if (!m_contactIMAccountResource.resourceUri().isEmpty()) {
                // Remove the publishesPresenceTo property
                if (m_contactIMAccountResource.publishesPresenceTos().contains(m_accountResource)) {
                    // FIXME: The correct code should be:
                    // m_contactIMAccountResource.removeProperty(Nepomuk::Vocabulary::Telepathy::publishesPresenceTo(),
                    //                                           m_accountResource);
                    // but it does not work
                    m_contactIMAccountResource.removeProperty(Nepomuk::Vocabulary::Telepathy::publishesPresenceTo());
                }
            }
            kDebug() << "State no, now it publishes to " << m_contactIMAccountResource.publishesPresenceTos();

            break;
        case Tp::Contact::PresenceStateAsk:
            // Do nothing here
            break;
    }
}

void TelepathyContact::onSubscriptionStateChanged(Tp::Contact::PresenceState state)
{
    kDebug() << "Subscription changed!";
    using namespace Nepomuk::Vocabulary;
    switch (state) {
        case Tp::Contact::PresenceStateYes:
            // We are now subscribed.

            // Only update the properties if we already have the contactIMAccountResource.
            if (!m_contactIMAccountResource.resourceUri().isEmpty()) {
                // If we're not yet isBuddyOf, let's become one.
                if (!m_contactIMAccountResource.isBuddyOfs().contains(m_accountResource)) {
                    kDebug() << "We're not yet buddies with the account";
                    m_contactIMAccountResource.addIsBuddyOf(m_accountResource);
                }
                // If there was a request, clear it
                if (m_contactIMAccountResource.requestedSubscriptionTos().contains(m_accountResource)) {
                    // Remove the publishesPresenceTo property
                    // FIXME: The correct code should be:
                    // m_contactIMAccountResource.removeProperty(Telepathy::requestedSubscriptionTo(),
                    //                                           m_accountResource);
                    // but it does not work
                    m_contactIMAccountResource.removeProperty(Telepathy::requestedSubscriptionTo());
                }
            }
            kDebug() << "State yes, now it's buddy of " << m_contactIMAccountResource.isBuddyOfs();

            break;
        case Tp::Contact::PresenceStateNo:
            // We are no longer subscribed. This means the user explicitely requested to clear
            // the contact from his buddy list. Let's do it then.

            // Only update the properties if we already have the contactIMAccountResource.
            if (!m_contactIMAccountResource.resourceUri().isEmpty()) {
                // Remove the isBuddyOf property, which will magically clear out the contact from our list
                if (m_contactIMAccountResource.isBuddyOfs().contains(m_accountResource)) {
                    // FIXME: The correct code should be:
                    // m_contactIMAccountResource.removeProperty(Telepathy::isBuddyOf(),
                    //                                           m_accountResource);
                    // but it does not work
                    m_contactIMAccountResource.removeProperty(Telepathy::isBuddyOf());
                }

                // If there was a request, clear it
                if (m_contactIMAccountResource.requestedSubscriptionTos().contains(m_accountResource)) {
                    // Remove the publishesPresenceTo property
                    // FIXME: The correct code should be:
                    // m_contactIMAccountResource.removeProperty(Telepathy::requestedSubscriptionTo(),
                    //                                           m_accountResource);
                    // but it does not work
                    m_contactIMAccountResource.removeProperty(Telepathy::requestedSubscriptionTo());
                }
            }
            kDebug() << "State no, now it's buddy of " << m_contactIMAccountResource.isBuddyOfs();

            break;
        case Tp::Contact::PresenceStateAsk:
            // The contact asked to subscribe to us

             // Only update the properties if we already have the contactIMAccountResource.
            if (!m_contactIMAccountResource.resourceUri().isEmpty()) {
                // If there was a request, clear it
                if (!m_contactIMAccountResource.requestedSubscriptionTos().contains(m_accountResource)) {
                    // Add the request
                    m_contactIMAccountResource.addRequestedSubscriptionTo(m_accountResource);
                }
            }
            kDebug() << "State ask, now it has requests towards "
                     << m_contactIMAccountResource.requestedSubscriptionTos();

            break;
    }
}

void TelepathyContact::onCapabilitiesChanged(Tp::ContactCapabilities* capabilities)
{
    QList< Nepomuk::Resource > imCapabilities = m_contactIMAccountResource.imCapabilitys();

    // Process all the various capabilities
    imCapabilities = processCapability(Nepomuk::Vocabulary::Telepathy::imcapabilityaudiocalls(),
                                       capabilities->supportsAudioCalls(),
                                       imCapabilities);
    imCapabilities = processCapability(Nepomuk::Vocabulary::Telepathy::imcapabilitytextchat(),
                                       capabilities->supportsTextChats(),
                                       imCapabilities);
    imCapabilities = processCapability(Nepomuk::Vocabulary::Telepathy::imcapabilityvideocalls(),
                                       capabilities->supportsVideoCalls(),
                                       imCapabilities);
    imCapabilities = processCapability(Nepomuk::Vocabulary::Telepathy::imcapabilityupgradingcalls(),
                                       capabilities->supportsUpgradingCalls(),
                                       imCapabilities);

    // Set them onto the resource now
    m_contactIMAccountResource.setImCapabilitys(imCapabilities);

    kDebug() << "Capabilities changed: "
             << capabilities->supportsTextChats()
             << capabilities->supportsAudioCalls()
             << capabilities->supportsVideoCalls()
             << capabilities->supportsUpgradingCalls();
}

QList< Nepomuk::Resource > TelepathyContact::processCapability(
                const QUrl& capability,
                bool isSupported,
                const QList< Nepomuk::Resource >& resources)
{
    QList< Nepomuk::Resource > imCapabilities = resources;
    if (isSupported) {
        bool found = false;
        foreach (const Nepomuk::Resource &cap, imCapabilities) {
            if (cap.hasType(capability)) {
                found = true;
                break;
            }
        }
        if (!found) {
            Nepomuk::Resource res;
            res.setTypes(QList< QUrl >() << capability);
            imCapabilities << res;
        }
    } else {
        QList< Nepomuk::Resource >::iterator it = imCapabilities.begin();
        while (it != imCapabilities.end()) {
            if ((*it).hasType(capability)) {
                it = imCapabilities.erase(it);
            } else {
                ++it;
            }
        }
    }

    return imCapabilities;
}

#include "telepathycontact.moc"

