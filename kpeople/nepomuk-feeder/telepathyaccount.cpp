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

// Ontology Vocabularies
#include "nco.h"
#include "telepathy.h"

#include <KDebug>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>

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
             << Tp::Account::FeatureProtocolInfo;

    connect(m_account->becomeReady(features),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountReady(Tp::PendingOperation*)));
/*
    // Connect to all the signals that indicate changes in properties we care about.
    connect(m_account.data(),
            SIGNAL(currentPresenceChanged(Tp::SimplePresence)),
            SLOT(onCurrentPresenceChanged(Tp::SimplePresence)));
    connect(m_account.data(),
            SIGNAL(displayNameChanged(QString)),
            SLOT(onDisplayNameChanged(QString)));
            // ...... and any other properties we want to sync...
*/
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

    // Check that this Account is set up in nepomuk.
    doNepomukSetup();

    // Connect to signals that indicate the account is online.
    connect(m_account.data(),
            SIGNAL(haveConnectionChanged(bool)),
            SLOT(onHaveConnectionChanged(bool)));
}

void TelepathyAccount::doNepomukSetup()
{
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
        kDebug() << "Found IM Account: " << foundImAccount.uri();

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
        kDebug() << "Account Identifier:" << accountIdentifier;

        if (accountIdentifier == m_path) {
            kDebug() << "Found the corresponding IMAccount in Nepomuk.";
                // It matches, so set our member variable to it and stop looping.
                m_accountResource = foundImAccount;
                break;
        }
    }

    // If the accountResource is still empty, create a new IMAccount.
    if (m_accountResource == Nepomuk::IMAccount()) {
        kDebug() << "Could not find corresponding IMAccount in Nepomuk. Creating a new one.";

        m_accountResource.addProperty(Nepomuk::Vocabulary::NCO::imAccountType(),
                                      m_account->protocol());
        m_accountResource.addProperty(Nepomuk::Vocabulary::NCO::imID(),
                                      m_account->parameters().value("account").toString());
        m_accountResource.addProperty(Nepomuk::Vocabulary::NCO::imNickname(),
                                      m_account->displayName());
        m_accountResource.addProperty(Nepomuk::Vocabulary::Telepathy::accountIdentifier(),
                                      m_path);

        mePersonContact.addProperty(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                                    m_accountResource);
    }
}

void TelepathyAccount::onHaveConnectionChanged(bool haveConnection)
{
    /*
    if (haveConnection) {
        // We now have a connection to the account. Get the connection ready to use.
        if (!m_connection.isNull()) {
            kWarning() << "Connection should be null, but is not :/";
            return;
        }

        m_connection = m_account->connection();
        
        Tp::Features features;
        features << Tp::Connection::FeatureCore
                 << Tp::Connection::FeatureSimplePresence
                 << Tp::Connection::FeatureSelfContact
                 << Tp::Connection::FeatureRoster;
        
        connect(m_connection->becomeReady(features),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onConnectionReady(Tp::PendingOperation*)));
    } else {
        // Connection has gone down. Delete our pointer to it.
        m_connection.reset();
    }
    */
}

void TelepathyAccount::onConnectionReady(Tp::PendingOperation *op)
{
    /*
    if (op->isError()) {
        kWarning() << "Getting connection ready failed.";
        m_connection.reset();
        return;
    }

    // TODO: Implement me!
    */
}


#include "telepathyaccount.moc"

