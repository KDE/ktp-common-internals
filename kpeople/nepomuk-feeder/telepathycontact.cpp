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
            SIGNAL(invalidated(Tp::DBusProxy*, const QString&, const QString&)),
            SLOT(deleteLater()));

    // Find the Nepomuk resource for this contact, or create a new one if necessary.
    doNepomukSetup();
}

TelepathyContact::~TelepathyContact()
{
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
        kDebug() << "Found IM Account: " << foundImAccount.uri();
        Nepomuk::IMAccount foundPersonContact(it.binding("b").uri());
        kDebug() << "Person Contact: " << foundPersonContact.uri();

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
        kDebug() << "Account ID:" << accountID;

        if (accountID == m_contact->id()) {
            kDebug() << "Found the corresponding IMAccount in Nepomuk.";
                // It matches, so set our member variables to found resources and stop looping.
                m_contactIMAccountResource = foundImAccount;
                m_contactPersonContactResource = foundPersonContact;

                // Sync any properties that may have changed since last time we were online.
                if (m_contactIMAccountResource.property(Nepomuk::Vocabulary::NCO::imNickname())
                    != m_contact->alias()) {
                    //onNicknameChanged(m_account->nickname());
                }
                //onCurrentPresenceChanged(m_account->currentPresence()); // We can always assume this one needs syncing.
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

        m_contactPersonContactResource.addProperty(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                                                   m_contactIMAccountResource);
        // FIXME: Store any other relevant Contact properties to Nepomuk.
    }
}


#include "telepathycontact.moc"

