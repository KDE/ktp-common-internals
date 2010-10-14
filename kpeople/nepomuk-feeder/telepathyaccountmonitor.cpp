/*
 * This file is part of nepomuktelepathyservice
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

#include "telepathyaccountmonitor.h"

// Ontology Vocabularies
#include "ontologies/nco.h"
#include "ontologies/pimo.h"

#include <KDebug>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Thing>

#include <QtCore/QString>

#include <TelepathyQt4/PendingReady>

TelepathyAccountMonitor::TelepathyAccountMonitor(QObject *parent)
 : QObject(parent),
   m_resourceManager(0)
{
    // Create an instance of the AccountManager and start to get it ready.
    m_accountManager = Tp::AccountManager::create();

    connect(m_accountManager->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    // Create an instance of the Nepomuk Resource Manager, and connect to it's error signal.
    m_resourceManager = Nepomuk::ResourceManager::instance();

    connect(m_resourceManager,
            SIGNAL(error(QString, int)),
            SLOT(onNepomukError(QString, int)));

    // Now do the initialization stuff for Nepomuk.
    doNepomukSetup();
}

TelepathyAccountMonitor::~TelepathyAccountMonitor()
{
    // Don't delete the Nepomuk Resource manager. Nepomuk should take care of this itself.
}

void TelepathyAccountMonitor::doNepomukSetup()
{
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

void TelepathyAccountMonitor::onAccountManagerReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Account manager cannot become ready:"
                   << op->errorName()
                   << op->errorMessage();
        return;
    }

     // Account Manager is now ready. We should watch for any new accounts being created.
    connect(m_accountManager.data(),
            SIGNAL(accountCreated(const QString&)),
            SLOT(onAccountCreated(const QString&)));

    // Take into account (ha ha) the accounts that already existed when the AM object became ready.
    foreach (const QString &path, m_accountManager->allAccountPaths()) {
         onAccountCreated(path);
     }
}

void TelepathyAccountMonitor::onAccountCreated(const QString &path)
{
    new TelepathyAccount(path, this);
}

Tp::AccountManagerPtr TelepathyAccountMonitor::accountManager() const
{
    return m_accountManager;
}

Nepomuk::PersonContact TelepathyAccountMonitor::mePersonContact() const
{
    return m_mePersonContact;
}

void TelepathyAccountMonitor::onNepomukError(const QString &uri, int errorCode)
{
    kWarning() << "A Nepomuk Error occurred:" << uri << errorCode;
}


#include "telepathyaccountmonitor.moc"

