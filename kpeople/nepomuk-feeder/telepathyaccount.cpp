/*
 * This file is part of telepathy-integration-daemon
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

#include "telepathyaccount.h"

#include "telepathyaccountmonitor.h"
#include "telepathycontact.h"

// Ontology Vocabularies
#include "nco.h"
#include "telepathy.h"

#include <informationelement.h>
#include <dataobject.h>

#include <KDebug>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>
#include <Nepomuk/Resource>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>

#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>

TelepathyAccount::TelepathyAccount(const QString &path, TelepathyAccountMonitor *parent)
 : QObject(parent),
   m_parent(parent),
   m_path(path)
{
    kDebug() << "Creating TelepathyAccount: " << path;

    // We need to get the Tp::Account ready before we do any other stuff.
    m_account = m_parent->accountManager()->accountForPath(path);

    Tp::Features features;
    features << Tp::Account::FeatureCore
             << Tp::Account::FeatureProtocolInfo
             << Tp::Account::FeatureAvatar;

    connect(m_account->becomeReady(features),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountReady(Tp::PendingOperation*)));

    // Connect to all the signals that indicate changes in properties we care about.
    connect(m_account.data(),
            SIGNAL(nicknameChanged(const QString &)),
            SLOT(onNicknameChanged(const QString &)));
    connect(m_account.data(),
            SIGNAL(currentPresenceChanged(Tp::SimplePresence)),
            SLOT(onCurrentPresenceChanged(Tp::SimplePresence)));
    connect(m_account.data(),
            SIGNAL(avatarChanged(Tp::Avatar)),
            SLOT(onAvatarChanged(Tp::Avatar)));
            // ...... and any other properties we want to sync...
}

TelepathyAccount::~TelepathyAccount()
{
}

void TelepathyAccount::onAccountReady(Tp::PendingOperation *op)
{
   if (op->isError()) {
        kWarning() << "Account"
                   << m_path
                   << "cannot become ready:"
                   << op->errorName()
                   << "-"
                   << op->errorMessage();
        return;
    }

    kDebug() << this;

    // Check that this Account is set up in nepomuk.
    doNepomukSetup();

    // Connect to signals that indicate the account is online.
    connect(m_account.data(),
            SIGNAL(haveConnectionChanged(bool)),
            SLOT(onHaveConnectionChanged(bool)));
}

void TelepathyAccount::doNepomukSetup()
{
    kDebug() << this;
    // Get a copy of the "me" PersonContact.
    Nepomuk::PersonContact mePersonContact = m_parent->mePersonContact();

    // Query Nepomuk for all IMAccounts that the "me" PersonContact has.
    QString query = QString("select distinct ?a where { %1 %2 ?a . ?a a %3 }")
                            .arg(Soprano::Node::resourceToN3(mePersonContact.resourceUri()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::hasIMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::IMAccount()));

    Soprano::Model *model = Nepomuk::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    // Iterate over all the IMAccounts found.
    while(it.next()) {
        Nepomuk::IMAccount foundImAccount(it.binding("a").uri());
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

        // Exactly one identifier found. Check if it matches the one we are looking for.
        QString accountIdentifier = accountIdentifiers.first();
        kDebug() << this << ": Account Identifier:" << accountIdentifier;

        if (accountIdentifier == m_path) {
            kDebug() << this << ": Found the corresponding IMAccount in Nepomuk.";
                // It matches, so set our member variable to it and stop looping.
                m_accountResource = foundImAccount;

                // Sync any properties that have changed on the AM behind our back.
                if (m_accountResource.property(Nepomuk::Vocabulary::NCO::imNickname()) != m_account->nickname()) {
                    onNicknameChanged(m_account->nickname());
                }
                onCurrentPresenceChanged(m_account->currentPresence()); // We can always assume this one needs syncing.
                // FIXME: Can Protocol and account properties change?

                break;
        }
    }

    // If the accountResource is still empty, create a new IMAccount.
    if (m_accountResource.uri().isEmpty()) {
        kDebug() << this << ": Could not find corresponding IMAccount in Nepomuk. Creating a new one.";

        m_accountResource.addProperty(Nepomuk::Vocabulary::NCO::imAccountType(),
                                      m_account->protocol());
        // FIXME: Some IM Accounts don't have an ID as such, e.g. Link-Local-XMPP.
        m_accountResource.addProperty(Nepomuk::Vocabulary::NCO::imID(),
                                      m_account->parameters().value("account").toString());
        m_accountResource.addProperty(Nepomuk::Vocabulary::NCO::imNickname(),
                                      m_account->nickname());
        m_accountResource.addProperty(Nepomuk::Vocabulary::NCO::imStatus(),
                                      m_account->currentPresence().status);
        m_accountResource.addProperty(Nepomuk::Vocabulary::NCO::imStatusMessage(),
                                      m_account->currentPresence().statusMessage);
        m_accountResource.addProperty(Nepomuk::Vocabulary::Telepathy::statusType(),
                                      m_account->currentPresence().type);
        m_accountResource.addProperty(Nepomuk::Vocabulary::Telepathy::accountIdentifier(),
                                      m_path);

        Nepomuk::InformationElement photo;
        photo.setPlainTextContents(QStringList() << m_account->avatar().avatarData.toBase64());
        Nepomuk::DataObject dataObject(m_accountResource);
        dataObject.addInterpretedAs(photo);
        m_accountResource.addProperty(Nepomuk::Vocabulary::NCO::photo(),
                                      dataObject);

        mePersonContact.addProperty(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                                     m_accountResource);
    }

    // Check if the account already has a connection, and
    // synthesise a connection state change if it has.
    if (m_account->haveConnection()) {
        onHaveConnectionChanged(true);
    }
}

void TelepathyAccount::onHaveConnectionChanged(bool haveConnection)
{
    if (haveConnection) {
        // We now have a connection to the account. Get the connection ready to use.
        if (!m_connection.isNull()) {
            kWarning() << "Connection should be null, but is not :/ Do nowt.";
            return;
        }

        m_connection = m_account->connection();

        Tp::Features features;
        features << Tp::Connection::FeatureCore
                 << Tp::Connection::FeatureSimplePresence
                 << Tp::Connection::FeatureSelfContact
                 << Tp::Connection::FeatureRoster
                 << Tp::Connection::FeatureRosterGroups;

        connect(m_connection->becomeReady(features),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onConnectionReady(Tp::PendingOperation*)));
    } else {
        // Connection has gone down. Delete our pointer to it.
        m_connection.reset();
    }
}

void TelepathyAccount::onConnectionReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Getting connection ready failed."
                   << op->errorName()
                   << op->errorMessage();
        m_connection.reset();
        return;
    }

    // We have a ready connection. Get all the contact list, and upgrade the contacts to have
    // all the features we want to store in Nepomuk.
    if (!m_connection->contactManager()) {
        kWarning() << "ContactManager is Null. Abort getting contacts.";
        return;
    }

    Tp::Contacts contacts = m_connection->contactManager()->allKnownContacts();

    QSet<Tp::Contact::Feature> features;
    features << Tp::Contact::FeatureAlias
             << Tp::Contact::FeatureSimplePresence
             << Tp::Contact::FeatureAvatarToken;

    // Handle contact avatars here, by using AvatarsInterface
    connect(m_connection.data()->avatarsInterface(),
            SIGNAL(AvatarRetrieved(uint,QString,QByteArray,QString)),
            SLOT(onContactAvatarRetrieved(uint,QString,QByteArray,QString)));
    connect(m_connection.data()->avatarsInterface(),
            SIGNAL(AvatarUpdated(uint,QString)),
            SLOT(onContactAvatarUpdated(uint,QString)));

    // Retrieve contacts
    connect(m_connection->contactManager()->upgradeContacts(contacts.toList(), features),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactsUpgraded(Tp::PendingOperation*)));
}

void TelepathyAccount::onContactsUpgraded(Tp::PendingOperation* op)
{
    if (op->isError()) {
        kWarning() << "Upgrading contacts failed."
                   << op->errorName()
                   << op->errorMessage();
        return;
    }

    Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);

    if (!pc) {
        kWarning() << "Casting to Tp::PendingContacts failed. Abort.";
        return;
    }

    // We have an upgraded contact list. Now we can create a TelepathyContact instance for
    // each contact.
    // We also keep tracks of handles to do avatar retrieval (if needed)
    Tp::UIntList avatarsToRetrieve;
    foreach (Tp::ContactPtr contact, pc->contacts()) {
        TelepathyContact *tpcontact = new TelepathyContact(contact, m_connection, m_accountResource, this);
        m_contacts.insert(contact, tpcontact);

        // We need to connect to the TelepathyContact's destroyed signal to remove it from the hash
        // when it is detroyed.
        connect(tpcontact,
                SIGNAL(contactDestroyed(Tp::ContactPtr)),
                SLOT(onContactDestroyed(Tp::ContactPtr)));

        if (tpcontact->avatarToken().isEmpty()) {
            // We totally need to retrieve the avatar
            avatarsToRetrieve << contact->handle().toList();
        } else {
            kDebug() << "Our contact already has the avatar " << tpcontact->avatarToken();
        }
    }

    if (!avatarsToRetrieve.isEmpty()) {
        kDebug() << "Pulling avatars";
        // Ok, pull the avatars here
        QDBusPendingReply< Tp::AvatarTokenMap > reply =
            m_connection.data()->avatarsInterface()->GetKnownAvatarTokens(avatarsToRetrieve);

        reply.waitForFinished();
        if (!reply.value().isEmpty()) {
            Tp::AvatarTokenMap result = reply.value();
            for (Tp::AvatarTokenMap::const_iterator i = result.constBegin(); i != result.constEnd(); ++i) {
                if (!i.value().isEmpty()) {
                    onContactAvatarUpdated(i.key(), i.value());
                }
            }
        }
    }
}

void TelepathyAccount::onNicknameChanged(const QString& nickname)
{
    // Do not update any property on the account resource if it hasn't yet been created.
    if (!m_accountResource.uri().isEmpty()) {
        m_accountResource.setProperty(Nepomuk::Vocabulary::NCO::imNickname(),
                                      nickname);
    }
}

void TelepathyAccount::onCurrentPresenceChanged(Tp::SimplePresence presence)
{
    // Do not update any property on the account resource if it hasn't yet been created.
    if (!m_accountResource.uri().isEmpty()) {
        m_accountResource.setProperty(Nepomuk::Vocabulary::NCO::imStatus(),
                                      presence.status);
        m_accountResource.setProperty(Nepomuk::Vocabulary::NCO::imStatusMessage(),
                                      presence.statusMessage);
        m_accountResource.setProperty(Nepomuk::Vocabulary::Telepathy::statusType(),
                                      presence.type);
    }
}

void TelepathyAccount::onAvatarChanged(const Tp::Avatar& avatar)
{
    // Do not update any property on the account resource if it hasn't yet been created.
    if (!m_accountResource.uri().isEmpty()) {
        // Store avatarData only. QImage will take care of determining the mimetype.
        Nepomuk::InformationElement photo;
        photo.setPlainTextContents(QStringList() << avatar.avatarData.toBase64());
        Nepomuk::DataObject dataObject(m_accountResource);
        dataObject.addInterpretedAs(photo);
        m_accountResource.setProperty(Nepomuk::Vocabulary::NCO::photo(),
                                      dataObject);
    }
}
void TelepathyAccount::onContactAvatarRetrieved(uint contact, const QString& token, const QByteArray& avatar, const QString&)
{
    kDebug() << "Avatar retrieved" << contact << token;
    // Retrieve the contact
    Tp::ContactPtr tpContact = m_connection.data()->contactManager()->lookupContactByHandle(contact);

    // Set the avatar
    TelepathyContact *telepathyContact = m_contacts[tpContact];
    if (telepathyContact) {
        // Match. Let's set the avatar then
        telepathyContact->setAvatar(token, avatar);
    }
}

void TelepathyAccount::onContactAvatarUpdated(uint contact, const QString& token)
{
    kDebug() << "Avatar updated" << token << contact;
    Tp::ContactPtr tpContact = m_connection.data()->contactManager()->lookupContactByHandle(contact);

    // Now we need to check if we need to update the avatar
    TelepathyContact *telepathyContact = m_contacts[tpContact];
    if (telepathyContact) {
        kDebug() << "match";
        // Match. Now check if we need to retrieve the avatar
        if (telepathyContact->avatarToken() != token) {
            kDebug() << "retrieve";
            // We do
            QDBusPendingReply<void> reply = m_connection.data()->avatarsInterface()->RequestAvatars(tpContact->handle().toList());
            if (reply.error().type() != QDBusError::NoError) {
                // TODO: What to do in case of error?
            }
        }
    }
}

void TelepathyAccount::onContactDestroyed(const Tp::ContactPtr &contact)
{
    if (!contact.isNull()) {
        m_contacts.remove(contact);
    }
}


#include "telepathyaccount.moc"

