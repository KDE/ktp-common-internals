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

#include "telepathy.h"

// Ontology uri's
#include "nco.h"
#include "pimo.h"

// Full Ontologies
#include "personcontact.h"
#include "imaccount.h"

#include <kdebug.h>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Thing>
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
    Nepomuk::PersonContact mePersonContact = m_parent->mePersonContact();
    Nepomuk::IMAccount imAccount;

    imAccount = getNepomukImAccount(mePersonContact);

    // If the IMAccount returned is empty, create a new one.
    if (imAccount == Nepomuk::IMAccount()) {
        kDebug() << "No Nepomuk::IMAccount found. Create a new one";

        imAccount.addProperty(Nepomuk::Vocabulary::NCO::imAccountType(), m_account->protocol());
        imAccount.addProperty(Nepomuk::Vocabulary::NCO::imID(), m_account->parameters().value("account").toString());
        imAccount.addProperty(Nepomuk::Vocabulary::NCO::imNickname(), m_account->displayName());
        imAccount.addProperty(Nepomuk::Vocabulary::Telepathy::accountIdentifier(), m_path);

        mePersonContact.addProperty(Nepomuk::Vocabulary::NCO::hasIMAccount(), imAccount);
    }
}

Nepomuk::IMAccount TelepathyAccount::getNepomukImAccount(const Nepomuk::PersonContact &mePersonContact)
{
    // ************************************************************************
    // Now we have got hold of "me", we
    // can query for "my" IM Accounts.
    QString query = QString("select distinct ?a where { %1 %2 ?a . ?a a %3 }")
            .arg(Soprano::Node::resourceToN3(mePersonContact.resourceUri()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::hasIMAccount()))
            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::IMAccount()));

    Soprano::Model *model = Nepomuk::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    while(it.next()) {
        Nepomuk::Resource foundImAccountResource(it.binding("a").uri());
        Nepomuk::IMAccount foundImAccount(foundImAccountResource);
        kDebug() << "Found IM Account: " << foundImAccount;

        QStringList accountIdentifiers = foundImAccount.accountIdentifiers();

        if (accountIdentifiers.size() != 0) {
            QString accountIdentifier = accountIdentifiers.first();

            kDebug() << "Account Identifier:" << accountIdentifier;

            if (accountIdentifier == m_path) {
                kDebug() << "Already have this account in Nepomuk. Skip.";
                // TODO: Update the account if necessary.
                return foundImAccount;
            }
        }
    }

    return Nepomuk::IMAccount();
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

