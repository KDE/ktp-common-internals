/*
 * This file is part of telepathy-integration-daemon
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

// Ontology Vocabularies
#include "nco.h"
#include "telepathy.h"

// Full Resource Classes
#include "contactgroup.h"
#include "dataobject.h"

#include <KDebug>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>

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
    // FIXME: Connect to any other signals of sync-worthy properties here.

    // Find the Nepomuk resource for this contact, or create a new one if necessary.
    doNepomukSetup();
}

TelepathyContact::~TelepathyContact()
{
    // Remove from cache
    m_parent->removeContact(m_contact);
    kDebug();
}

void TelepathyContact::doNepomukSetup()
{
    // Query Nepomuk for all IM accounts that isBuddyOf the accountResource
    QString query = QString("select distinct ?a ?b where { ?a %1 %2 . ?a a %3 . ?b %4 ?a . ?b a %5}")
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::Telepathy::isBuddyOf()))
                            .arg(Soprano::Node::resourceToN3(m_accountResource.uri()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::IMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::hasIMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::PersonContact()));

    Soprano::Model *model = Nepomuk::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    // Iterate over all the IMAccounts found.
    while(it.next()) {
        Nepomuk::IMAccount foundImAccount(it.binding("a").uri());
        Nepomuk::IMAccount foundPersonContact(it.binding("b").uri());

        // Check that the IM account only has one ID.
        QStringList accountIDs = foundImAccount.imIDs();

        if (accountIDs.size() != 1) {
            kDebug() << "Account does not have 1 ID. Oops. Ignoring."
                     << "Number of Identifiers: "
                     << accountIDs.size();
                     continue;
        }

        // Exactly one ID found. Check if it matches the one we are looking for.
        QString accountID = accountIDs.first();

        if (accountID == m_contact->id()) {
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
    }

    // If the contactIMAccountResource is still empty, create a new IMAccount and PersonContact.
    if (m_contactIMAccountResource.uri().isEmpty()) {
        kDebug() << this << ": Create new IM Account and corresponding PersonContact";

        m_contactIMAccountResource.addProperty(Nepomuk::Vocabulary::NCO::imID(),
                                               m_contact->id());
        m_contactIMAccountResource.addProperty(Nepomuk::Vocabulary::Telepathy::isBuddyOf(),
                                               m_accountResource);
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
    // Only update the properties if we already have the contactIMAccountResource.
    if (!m_contactIMAccountResource.resourceUri().isEmpty()) {

        // Check if the contact is already in that group
        foreach(Nepomuk::ContactGroup g, m_contactPersonContactResource.belongsToGroups()) {
            if (g.contactGroupName() == group) {
                // Already in that group
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
        m_contactPersonContactResource.addBelongsToGroup(groupResource);
    }
}

void TelepathyContact::onRemovedFromGroup(const QString &group)
{
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



#include "telepathycontact.moc"

