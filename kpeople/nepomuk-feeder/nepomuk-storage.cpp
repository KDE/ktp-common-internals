/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2009-2011 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
 *
 * Copyright (C) 2011 Vishesh Handa <handa.vish@gmail.com>
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
#include "ontologies/telepathy.h"
#include "ontologies/imcapability.h"
#include "ontologies/personcontact.h"
#include "ontologies/dataobject.h"

#include <KDebug>
#include <KJob>

#include <nepomuk/datamanagement.h>
#include <nepomuk/storeresourcesjob.h>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <nepomuk/simpleresource.h>
#include <nepomuk/simpleresourcegraph.h>
#include <Nepomuk/Thing>
#include <Nepomuk/Variant>

#include <Nepomuk/Vocabulary/NCO>
#include <Nepomuk/Vocabulary/PIMO>

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
#include <QtCore/QTimer>

#include <TelepathyQt/Constants>
#include <TelepathyQt/AvatarData>
#include <Soprano/Vocabulary/NAO>

using namespace Nepomuk::Vocabulary;

class AccountResources::Data : public QSharedData {
public:
    Data(const QUrl &a, const QString &p)
    : account(a),
      protocol(p)
    { }

    Data()
    { }

    Data(const Data &other)
    : QSharedData(other),
      account(other.account),
      protocol(other.protocol)
    { }

    ~Data()
    { }

    QUrl account;
    QString protocol;
};

AccountResources::AccountResources(const QUrl &account,
                                   const QString &protocol)
  : d(new Data(account, protocol))
{ }

AccountResources::AccountResources()
  : d(new Data())
{ }

AccountResources::AccountResources(const AccountResources &other)
  : d(other.d)
{ }

AccountResources::AccountResources(const QUrl &url)
  : d(new Data(url, QString()))
{ }

AccountResources::~AccountResources()
{ }

const QUrl &AccountResources::account() const
{
    return d->account;
}

const QString &AccountResources::protocol() const
{
    return d->protocol;
}

bool AccountResources::operator==(const AccountResources &other) const
{
    return (other.account() == account());
}

bool AccountResources::operator!=(const AccountResources &other) const
{
    return !(*this == other);
}

bool AccountResources::operator==(const QUrl &other) const
{
    return (other == account());
}

bool AccountResources::operator!=(const QUrl &other) const
{
    return !(*this == other);
}


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

    Data()
    { }

    ~Data()
    { }

    QString accountId;
    QString contactId;
};

ContactIdentifier::ContactIdentifier(const QString &accountId, const QString &contactId)
  : d(new Data(accountId, contactId))
{ }

ContactIdentifier::ContactIdentifier(const ContactIdentifier &other)
  : d(other.d)
{ }

ContactIdentifier::ContactIdentifier()
  : d(new Data())
{ }

ContactIdentifier::~ContactIdentifier()
{ }

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
    Data(const QUrl &p, const QUrl &pc,  const QUrl &ia)
      : person(p),
        personContact(pc),
        imAccount(ia)
    { }

    Data()
    { }

    Data(const Data &other)
      : QSharedData(other),
        person(other.person),
        personContact(other.personContact),
        imAccount(other.imAccount)
    { }

    ~Data()
    { }

    QUrl person;
    QUrl personContact;
    QUrl imAccount;
};

ContactResources::ContactResources(const QUrl &person,
                                   const QUrl &personContact,
                                   const QUrl &imAccount)
  : d(new Data(person, personContact, imAccount))
{ }

ContactResources::ContactResources()
  : d(new Data())
{ }

ContactResources::ContactResources(const ContactResources &other)
  : d(other.d)
{ }

ContactResources::~ContactResources()
{ }

const QUrl &ContactResources::person() const
{
    return d->person;
}

const QUrl &ContactResources::personContact() const
{
    return d->personContact;
}

const QUrl &ContactResources::imAccount() const
{
    return d->imAccount;
}

bool ContactResources::operator==(const ContactResources& other) const
{
    return ((other.person() == person()) &&
            (other.personContact() == personContact()) &&
            (other.imAccount() == imAccount()));
}

bool ContactResources::operator!=(const ContactResources& other) const
{
    return !(*this == other);
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

    QTimer::singleShot(0, this, SLOT(init()));
}

NepomukStorage::~NepomukStorage()
{
    // Don't delete the Nepomuk Resource manager. Nepomuk should take care of this itself.
    kDebug();
}

void NepomukStorage::onNepomukError(const QString &uri, int errorCode)
{
    kWarning() << "A Nepomuk Error occurred:" << uri << errorCode;
}

void NepomukStorage::onSaveJobResult(KJob *job)
{
    if (job->error()) {
        kWarning() << "Error Code:" << job->error() << job->errorString() << job->errorText();
        return;
    }

    kDebug() << "Save job succeeded.";
}

void NepomukStorage::init()
{
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
            m_mePersonContact = resource.resourceUri();
            break;
        }
    }

    if (!Nepomuk::Resource(m_mePersonContact).exists()) {
        kWarning() << "PersonContact 'me' does not exist. Creating it.";
        // FIXME: We shouldn't create this person contact, but for now we will
        // to ease development :) (see above FIXME's)
        m_mePersonContact = "nepomuk:/myself-person-contact";
        me.addGroundingOccurrence(m_mePersonContact);
    }

    // *********************************************************************************************
    // Load all the relevant accounts and contacts that are already in Nepomuk.

    // We load everything now, because this is much more efficient than re-running a query for each
    // and every account and contact individually when we get to that one.

    // Query Nepomuk for all of the ME PersonContact's IMAccounts.
    {
        using namespace Nepomuk::Query;

        // Construct the query
        ComparisonTerm accountTerm(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                                   ResourceTerm(m_mePersonContact));
        accountTerm.setInverted(true);

        ComparisonTerm hasTelepathyIdTerm(Nepomuk::Vocabulary::Telepathy::accountIdentifier(),
                                          LiteralTerm());
        hasTelepathyIdTerm.setVariableName("accountIdentifier");
        ComparisonTerm hasProtocolTerm(Nepomuk::Vocabulary::NCO::imAccountType(),
                                       LiteralTerm());
        hasProtocolTerm.setVariableName("protocol");

        Query query(AndTerm(accountTerm,
                            ResourceTypeTerm(Nepomuk::Vocabulary::NCO::IMAccount()),
                            hasTelepathyIdTerm, hasProtocolTerm));

        // Connect to the result signals and launch the query.
        QueryServiceClient *client = new QueryServiceClient(this);
        connect(client,
                SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
                SLOT(onAccountsQueryNewEntries(QList<Nepomuk::Query::Result>)));
        connect(client,
                SIGNAL(entriesRemoved(QList<QUrl>)),
                SLOT(onAccountsQueryEntriesRemoved(QList<QUrl>)));
        connect(client,
                SIGNAL(error(QString)),
                SLOT(onAccountsQueryError(QString)));
        connect(client,
                SIGNAL(finishedListing()),
                SLOT(onAccountsQueryFinishedListing()));
        kDebug() << "Me Query : " << query.toSparqlQuery();
        client->query(query);
    }
}

void NepomukStorage::onAccountsQueryNewEntries(const QList<Nepomuk::Query::Result> &entries)
{
    kDebug();
    // Iterate over all the IMAccounts found.
    foreach (const Nepomuk::Query::Result &result, entries) {
        QUrl foundImAccount(result.resource().resourceUri());
        kDebug() << this << ": Found IM Account: " << foundImAccount;

        // If no Telepathy identifier, then the account is ignored.
        QString foundImAccountIdentifier(result.additionalBinding("accountIdentifier").toString());
        if (foundImAccountIdentifier.isEmpty()) {
            kDebug() << "Account does not have a Telepathy Account Identifier. Oops. Ignoring.";
            continue;
        }

        kDebug() << "Found a Telepathy account in Nepomuk, ID:" << foundImAccountIdentifier;

        // If it does have a telepathy identifier, then it is added to the cache.
        m_accounts.insert(foundImAccountIdentifier,
                          AccountResources(foundImAccount,
                                           result.additionalBinding("protocol").toString()));
    }

}

void NepomukStorage::onAccountsQueryEntriesRemoved(const QList<QUrl> &entries)
{
    kDebug();
    // Remove the account from the cache
    foreach (const QUrl &url, entries) {
        m_accounts.remove(m_accounts.key(url));
    }
}

void NepomukStorage::onAccountsQueryError(const QString &errorMessage)
{
    kWarning() << "A Nepomuk Error occurred:" << errorMessage;

    emit initialised(false);
}

void NepomukStorage::onAccountsQueryFinishedListing()
{
    kDebug() << "Accounts Query Finished Successfully.";
    // Got all the accounts, now move on to the contacts.

    // Query Nepomuk for all know Contacts.
    {
        using namespace Nepomuk::Query;

        // Get the PIMO Person owning that PersonContact
        ComparisonTerm pterm(Nepomuk::Vocabulary::PIMO::groundingOccurrence(),
                             ResourceTypeTerm(Nepomuk::Vocabulary::PIMO::Person()));
        pterm.setVariableName("person");
        pterm.setInverted(true);

        // Get the person contact owning this IMAccount
        ComparisonTerm pcterm(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                            AndTerm(ResourceTypeTerm(Nepomuk::Vocabulary::NCO::PersonContact()),
                                    pterm));
        pcterm.setVariableName("personContact");
        pcterm.setInverted(true);

        // Special case: if we're buddy of an account we do own, we want to create a new
        // resource for that.
        // This avoids race conditions and a lot of bad things.
        ComparisonTerm accountTerm(Nepomuk::Vocabulary::NCO::hasIMAccount(),
                                ResourceTerm(m_mePersonContact));
        accountTerm.setInverted(true);

        ComparisonTerm hasTelepathyIdTerm(Nepomuk::Vocabulary::Telepathy::accountIdentifier(), LiteralTerm());
        hasTelepathyIdTerm.setVariableName("accountIdentifier");

        ComparisonTerm accessedByTerm(Nepomuk::Vocabulary::NCO::isAccessedBy(),
                                     AndTerm(ResourceTypeTerm(Nepomuk::Vocabulary::NCO::IMAccount()),
                                             hasTelepathyIdTerm));
        accessedByTerm.setVariableName("accessedBy");

        ComparisonTerm imIdsTerm(Nepomuk::Vocabulary::NCO::imID(), LiteralTerm());
        imIdsTerm.setVariableName("imIds");

        Query query(AndTerm(pcterm, NegationTerm::negateTerm(accountTerm), accessedByTerm,
                            ResourceTypeTerm(Nepomuk::Vocabulary::NCO::IMAccount()), imIdsTerm));

        QueryServiceClient *client = new QueryServiceClient(this);
        connect(client,
                SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
                SLOT(onContactsQueryNewEntries(QList<Nepomuk::Query::Result>)));
        connect(client,
                SIGNAL(entriesRemoved(QList<QUrl>)),
                SLOT(onContactsQueryEntriesRemoved(QList<QUrl>)));
        connect(client,
                SIGNAL(error(QString)),
                SLOT(onContactsQueryError(QString)));
        connect(client,
                SIGNAL(finishedListing()),
                SLOT(onContactsQueryFinishedListing()));
        client->query(query);
    }
}

void NepomukStorage::onContactsQueryNewEntries(const QList< Nepomuk::Query::Result > &entries)
{
    // Iterate over all the IMAccounts found.
    foreach (const Nepomuk::Query::Result &result, entries) {

        QUrl foundImAccount(result.resource().resourceUri());
        QUrl foundPersonContact(result.additionalBinding("personContact").toUrl());
        QUrl foundPerson(result.additionalBinding("person").toUrl());
        QUrl foundImAccountAccessedBy(result.additionalBinding("accessedBy").toUrl());
        QString foundImAccountIdentifier(result.additionalBinding("accountIdentifier").toString());

        kDebug() << "Account Identifier: " << foundImAccountIdentifier;
        kDebug() << "PIMO:Person" << foundPerson;

        // Check that the IM account only has one ID.
        QStringList accountIDs = result.additionalBinding("imIds").toStringList();

        if (accountIDs.size() != 1) {
            kWarning() << "Account does not have 1 ID. Oops. Ignoring."
            << "Number of Identifiers: "
            << accountIDs.size();
            continue;
        }

        kDebug() << "IM ID:" << accountIDs.first();

        // Cache the contact
        m_contacts.insert(ContactIdentifier(foundImAccountIdentifier,
                                            accountIDs.first()),
                            ContactResources(foundPerson, foundPersonContact, foundImAccount));
        kDebug() << "Contact found in Nepomuk. Caching.";
    }
}

void NepomukStorage::onContactsQueryEntriesRemoved(const QList<QUrl> &entries)
{
    foreach (const QUrl &entry, entries) {
        foreach (const ContactResources &resources, m_contacts.values()) {
            if (resources.personContact() == entry) {
                m_contacts.remove(m_contacts.key(resources));
                break;
            }
        }
    }
}

void NepomukStorage::onContactsQueryError(const QString &errorMessage)
{
    kWarning() << "A Nepomuk Error occurred:" << errorMessage;

    emit initialised(false);
}

void NepomukStorage::onContactsQueryFinishedListing()
{
    kDebug() << "Contacts Query Finished Successfully.";

    emit initialised(true);
}

void NepomukStorage::cleanupAccounts(const QList<QString> &paths)
{
    kDebug() << paths;
    kDebug() << "Our list: " << m_accounts.keys();

    QSet<QString> pathSet = paths.toSet();

    // Go through all our accounts and remove all the ones that are not there in the path
    // list given by the controller
    QList<QUrl> removedAccounts;
    QMutableHashIterator<QString, AccountResources> it( m_accounts );
    while( it.hasNext() ) {
        it.next();
        if (!pathSet.contains(it.key())) {
            removedAccounts << it.value().account();
            it.remove();
        }
    }

    // TODO: Do this properly once the ontology supports this
    // TODO: What do we do with an account in nepomuk which the use has removed?
    //       For now we're just deleting them
    KJob *job = Nepomuk::removeResources( removedAccounts, Nepomuk::RemoveSubResoures );
    job->exec();
    if( job->error() ) {
        kWarning() << job->errorString();
    }

    // Go through all the accounts that we have received from the controller and create any
    // new ones in neponmuk. Do this as a batch job to improve performance.
    foreach (const QString &path, paths) {
        if (!m_accounts.keys().contains(path)) {
            // TODO: Implement me to do this as a batch job???
            //       For now, we just let the constructed signal do this one at a time.
        }
    }
}

void NepomukStorage::saveGraph(const Nepomuk::SimpleResourceGraph &graph)
{
    KJob *job = Nepomuk::storeResources(graph, Nepomuk::IdentifyNew, Nepomuk::OverwriteProperties);
    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(onSaveJobResult(KJob*)));
    job->start();
}

void NepomukStorage::createAccount(const QString &path, const QString &id, const QString &protocol)
{
    kDebug() << "Creating a new Account";

    // First check if we already have this account.
    if (m_accounts.contains(path)) {
        kDebug() << "Account has already been created.";
        return;
    }

    kDebug() << "Could not find corresponding IMAccount in Nepomuk. Creating a new one.";

    // Add/Update relevant resources
    Nepomuk::SimpleResource imAccount;
    imAccount.addType(Nepomuk::Vocabulary::NCO::IMAccount());
    imAccount.addProperty(Nepomuk::Vocabulary::Telepathy::accountIdentifier(), path);
    imAccount.addProperty(Nepomuk::Vocabulary::NCO::imAccountType(), protocol);
    imAccount.addProperty(Nepomuk::Vocabulary::NCO::imID(), id);

    Nepomuk::SimpleResource mePersonContact(m_mePersonContact);
    mePersonContact.addProperty(Nepomuk::Vocabulary::NCO::hasIMAccount(), imAccount);

    // Add all changed resources to a graph and save it.
    Nepomuk::SimpleResourceGraph graph;
    graph << imAccount << mePersonContact;

    Nepomuk::StoreResourcesJob *job = Nepomuk::storeResources(graph);
    job->exec();
    if( job->error() ) {
        kWarning() << job->errorString();
        return;
    }

    QUrl accountUri = job->mappings().value( imAccount.uri() );
    Q_ASSERT(!accountUri.isEmpty());

    kDebug() << ">>>>>***** The Account URI is:" << accountUri;
    // Add the account to the list.
    m_accounts.insert(path, AccountResources(accountUri, protocol));
}

void NepomukStorage::destroyAccount(const QString &path)
{
    // The account object has been destroyed, which means we no longer know the presence of the
    // account, so it should be set to unknown.

    Tp::SimplePresence presence;
    presence.status = QLatin1String("unknown");
    presence.type = Tp::ConnectionPresenceTypeUnknown;

    setAccountCurrentPresence(path, presence);
}

void NepomukStorage::setAccountNickname(const QString &path, const QString &nickname)
{
    // Check the account exists
    QHash<QString, AccountResources>::const_iterator it = m_accounts.find(path);
    const bool found = (m_accounts.constEnd() != it);
    Q_ASSERT(found);
    if (!found) {
        kWarning() << "Account not found.";
        return;
    }

    QUrl accountUri = it.value().account();

    KJob *job = Nepomuk::setProperty(QList<QUrl>() << accountUri, NCO::imNickname(), QVariantList() << nickname);
    job->exec();
    if( job->error() ) {
        kWarning() << job->errorString();
    }
}

void NepomukStorage::setAccountCurrentPresence(const QString &path, const Tp::SimplePresence &presence)
{
    // Check the account exists
    QHash<QString, AccountResources>::const_iterator it = m_accounts.find(path);
    const bool found = (m_accounts.constEnd() != it);
    Q_ASSERT(found);
    if (!found) {
        kWarning() << "Account not found.";
        return;
    }

    QUrl accountUri = it.value().account();

    Nepomuk::SimpleResource account( accountUri );

    // Update the Presence properties.
    account.setProperty(Nepomuk::Vocabulary::NCO::imStatus(), presence.status);
    account.setProperty(Nepomuk::Vocabulary::NCO::imStatusMessage(), presence.statusMessage);
    account.setProperty(Nepomuk::Vocabulary::Telepathy::statusType(), presence.type);

    KJob *job = Nepomuk::storeResources(Nepomuk::SimpleResourceGraph() << account,
                                        Nepomuk::IdentifyNew, Nepomuk::OverwriteProperties );
    job->exec();
    if( job->error() ) {
        kWarning() << job->errorString();
    }
}

void NepomukStorage::cleanupAccountContacts(const QString &path, const QList<QString> &ids)
{
    kDebug() << path << ids;

    QSet<QString> idSet = ids.toSet();

    // Go through all the contacts in the cache and make any that are not in the list we
    // received from the account into Ghost Contacts. Do this as a batch job to improve performance.
    foreach (const ContactIdentifier &cid, m_contacts.keys()) {
        if (cid.accountId() == path) {
            if (!idSet.contains(cid.contactId())) {
                // TODO: Do this properly once the ontology supports this
                // TODO: Do this as a batch job to reduce the number of nepomuk queries that result.
                kDebug() << "Ghosting contact: " << cid.contactId();
                setContactPublishState(path, cid.contactId(), Tp::Contact::PresenceStateNo);
                setContactSubscriptionState(path, cid.contactId(), Tp::Contact::PresenceStateNo);
            }
        }
    }

    // Go through all the contacts that we have received from the account and create any
    // new ones in Nepomuk. Do this as a batch job to improve performance.
    QSet<QString> nepomukIds;
    foreach( const ContactIdentifier& ci, m_contacts.keys() )
        nepomukIds.insert( ci.contactId() );

    QSet<QString> newIds = idSet.subtract( nepomukIds );
    foreach( const QString& id, newIds ) {
        createContact(path, id);
    }
}

void NepomukStorage::createContact(const QString &path, const QString &id)
{
    kDebug() << path << id;
    // First, check that we don't already have a record for this contact.
    ContactIdentifier identifier(path, id);
    if (m_contacts.contains(identifier)) {
        kDebug() << "Contact record already exists. Return.";
        return;
    }

    // Contact not found. Need to create it.
    kDebug() << "Contact not found in Nepomuk. Creating it.";

    Q_ASSERT(m_accounts.contains(path));
    if (!m_accounts.contains(path)) {
        kWarning() << "Corresponding account not cached.";
        return;
    }

    AccountResources accountRes = m_accounts.value(path);
    QUrl accountUri = accountRes.account();

    Nepomuk::SimpleResource newPersonContact;
    newPersonContact.addType(Nepomuk::Vocabulary::NCO::PersonContact());

    Nepomuk::SimpleResource newPimoPerson;
    newPimoPerson.addType(Nepomuk::Vocabulary::PIMO::Person());

    Nepomuk::SimpleResource newImAccount;
    //TODO: Somehow add this imAccount as a sub resource the account, maybe.
    newImAccount.addType(Nepomuk::Vocabulary::NCO::IMAccount());
    newImAccount.setProperty(Nepomuk::Vocabulary::NCO::imStatus(), QString::fromLatin1("unknown"));
    newImAccount.setProperty(Nepomuk::Vocabulary::NCO::imID(), id);
    newImAccount.setProperty(Nepomuk::Vocabulary::Telepathy::statusType(), Tp::ConnectionPresenceTypeUnknown);
    newImAccount.setProperty(Nepomuk::Vocabulary::NCO::imAccountType(), accountRes.protocol());
    newImAccount.addProperty(Nepomuk::Vocabulary::NCO::isAccessedBy(), accountUri);

    newPersonContact.addProperty(Nepomuk::Vocabulary::NCO::hasIMAccount(), newImAccount);
    newPimoPerson.addProperty(Nepomuk::Vocabulary::PIMO::groundingOccurrence(), newPersonContact);

    Nepomuk::SimpleResourceGraph graph;
    graph << newPersonContact << newImAccount << newPimoPerson;

    Nepomuk::StoreResourcesJob *job = Nepomuk::storeResources( graph );
    job->exec();
    if( job->error() ) {
        kWarning() << job->errorString();
        return;
    }

    QHash< QUrl, QUrl > mappings = job->mappings();

    // Add it to the Contacts list.
    m_contacts.insert( identifier,
                       ContactResources(mappings.value(newPimoPerson.uri()),
                                        mappings.value(newPersonContact.uri()),
                                        mappings.value(newImAccount.uri())) );
}

void NepomukStorage::destroyContact(const QString &path, const QString &id)
{
    Tp::SimplePresence presence;
    presence.status = QLatin1String("unknown");
    presence.type = Tp::ConnectionPresenceTypeUnknown;

    setContactPresence(path, id, presence);
}

void NepomukStorage::setContactAlias(const QString &path, const QString &id, const QString &alias)
{
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    QHash<ContactIdentifier, ContactResources>::const_iterator it = m_contacts.find(identifier);
    const bool found = (it != m_contacts.constEnd());
    Q_ASSERT(found);
    if (!found) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = it.value();

    // vHanda: FIXME: Can a person have multiple nick names?
    KJob *job = Nepomuk::setProperty( QList<QUrl>() << resources.imAccount(), NCO::imNickname(),
                                      QVariantList() << alias );

    job->exec();
    if( job->error() ) {
        kWarning() << job->errorString();
    }

    if (!Nepomuk::Resource(resources.person()).hasProperty(Soprano::Vocabulary::NAO::prefLabel())) {
        KJob *job = Nepomuk::setProperty( QList<QUrl>() << resources.person(), Soprano::Vocabulary::NAO::prefLabel(),
                                          QVariantList() << alias );

        job->exec();
        if( job->error() ) {
            kWarning() << job->errorString();
        }
    }
}

void NepomukStorage::setContactPresence(const QString &path,
                                 const QString &id,
                                 const Tp::SimplePresence &presence)
{
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    QHash<ContactIdentifier, ContactResources>::const_iterator it = m_contacts.find(identifier);
    const bool found = (it != m_contacts.constEnd());
    Q_ASSERT(found);
    if (!found) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = it.value();

    Nepomuk::SimpleResource imAccount(resources.imAccount());
    // Set the contact presence.
    imAccount.setProperty(NCO::imStatus(), presence.status);
    imAccount.setProperty(Telepathy::statusType(), presence.type);
    imAccount.setProperty(NCO::imStatusMessage(), presence.statusMessage);

    // We're using 'OverwriteProperties' cause the above properties can already have existing
    // values which we don't care about
    KJob *job = Nepomuk::storeResources( Nepomuk::SimpleResourceGraph() << imAccount,
                                         Nepomuk::IdentifyNew, Nepomuk::OverwriteProperties );
    job->exec();
    if( job->error() ) {
        kWarning() << job->errorString();
    }
}

void NepomukStorage::setContactGroups(const QString &path,
                                      const QString &id,
                                      const QStringList &groups)
{
//     kDebug() << path << id << groups;
//     ContactIdentifier identifier(path, id);
//
//     // Check if the Contact exists.
//     QHash<ContactIdentifier, ContactResources>::const_iterator it = m_contacts.find(identifier);
//     const bool found = (it != m_contacts.constEnd());
//     Q_ASSERT(found);
//     if (!found) {
//         kWarning() << "Contact not found.";
//         return;
//     }
//
//     ContactResources resources = it.value();
//     Nepomuk::PersonContact personContact = resources.personContact();
//
//     // Set the contact groups.
//     // First remove any groups we are no longer a member of.
//     QList<Nepomuk::ContactGroup> newGroups = personContact.belongsToGroups();
//
//     foreach (const Nepomuk::ContactGroup &group, personContact.belongsToGroups()) {
//         if (!groups.contains(group.contactGroupName())) {
//             newGroups.removeAll(group);
//         }
//     }
//
//     // Now add any groups we are newly a member of.
//     bool found;
//     foreach (const QString &groupName, groups) {
//         found = false;
//         foreach (const Nepomuk::ContactGroup &cGroup, newGroups) {
//             if (cGroup.contactGroupName() == groupName) {
//                 found = true;
//                 break;
//             }
//         }
//
//         if (!found) {
//             // Not already in that group. Check the group exists.
//             // FIXME: Once we have a "ContactList" resource for Telepathy Contacts, we should only
//             //        get the groups associated with that.
//             Nepomuk::ContactGroup groupResource;
//             foreach (const Nepomuk::ContactGroup &g, Nepomuk::ContactGroup::allContactGroups()) {
//                 if (g.contactGroupName() == groupName) {
//                     groupResource = g;
//                     break;
//                 }
//             }
//
//             // If the group doesn't already exist, create it.
//             if (groupResource.resourceUri().isEmpty()) {
//                 // FIXME: Once we have a "ContactList" resource for Telepathy Contacts, we should
//                 //        create this group as a child of that resource.
//                 groupResource.setContactGroupName(groupName);
//             }
//
//             newGroups.append(groupResource);
//         }
//     }
//
//     // Update the groups property with the new list
//     personContact.setBelongsToGroups(newGroups);
//     kDebug() << "Set Groups Ending";
}

void NepomukStorage::setContactBlockStatus(const QString &path, const QString &id, bool blocked)
{
    ContactIdentifier identifier(path, id);

    // Check if the Contact exists.
    QHash<ContactIdentifier, ContactResources>::const_iterator it = m_contacts.find(identifier);
    const bool found = (it != m_contacts.constEnd());
    Q_ASSERT(found);
    if (!found) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = it.value();

    // vHanda: FIXME: Can a person have multiple nick names?
    KJob *job = Nepomuk::setProperty( QList<QUrl>() << resources.imAccount(), NCO::isBlocked(),
                                      QVariantList() << blocked );

    job->exec();
    if( job->error() ) {
        kWarning() << job->errorString();
    }
}

void NepomukStorage::setContactPublishState(const QString &path,
                                     const QString &id,
                                     const Tp::Contact::PresenceState &state)
{
    /*
    ContactIdentifier identifier(path, id);

    // Check if the Contact exists.
    QHash<ContactIdentifier, ContactResources>::const_iterator it = m_contacts.find(identifier);
    const bool found = (it != m_contacts.constEnd());
    Q_ASSERT(found);
    if (!found) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = it.value();

    // Get the local related account.
    Q_ASSERT(m_accounts.contains(path));
    if (!m_accounts.contains(path)) {
        kWarning() << "Account not found.";
        return;
    }

    QUrl imAccountUri = resources.imAccount();
    QUrl localAccountUri = m_accounts.value(path).account();*/

    //FIXME: Implment me properly using Nepomuk::Resource (when it is ported to the DMS)
    // Change the relationships based on the PresenceState value.
//     if (state == Tp::Contact::PresenceStateYes) {
//         // Add publishes to us.
//
//         KJob *job = Nepomuk::addProperty( NCO::publishesPresenceTo(), localAccountUri );
//         job->exec();
//         if( job->error() ) {
//             kWarning() << job->errorString();
//         }
//
//         // Remove requested from us.
//         localAccount.remove(Nepomuk::Vocabulary::NCO::requestedPresenceSubscriptionTo(), imAccount.uri());
//
//     } else if (state == Tp::Contact::PresenceStateAsk) {
//         // We request subscription to them
//         localAccount.addProperty(Nepomuk::Vocabulary::NCO::requestedPresenceSubscriptionTo(), localAccount.uri());
//
//         // Remove us from their publish list.
//         imAccount.remove(Nepomuk::Vocabulary::NCO::publishesPresenceTo(), localAccount.uri());
//
//     } else if (state == Tp::Contact::PresenceStateNo) {
//         // Remove us from the requested-to-them list
//         localAccount.remove(Nepomuk::Vocabulary::NCO::requestedPresenceSubscriptionTo(), imAccount.uri());
//
//         // Remove us from their publish list
//         imAccount.remove(Nepomuk::Vocabulary::NCO::publishesPresenceTo(), localAccount.uri());
//
//     } else {
//         kWarning() << "Invalid Tp::Contact::PresenceState received.";
//         Q_ASSERT(false);
//     }
}

void NepomukStorage::setContactSubscriptionState(const QString &path,
                                          const QString &id,
                                          const Tp::Contact::PresenceState &state)
{
//    kDebug() << "Not implemented";
//    kDebug() << path << id << state;
    /*
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    Q_ASSERT(m_contacts.contains(identifier));
    if (!m_contacts.contains(identifier)) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = m_contacts.value(identifier);

    Nepomuk::SimpleResource imAccount(resources.imAccount());

    // Get the local related account.
    Q_ASSERT(m_accounts.contains(path));
    if (!m_accounts.contains(path)) {
        kWarning() << "Account not found.";
        return;
    }

    Nepomuk::SimpleResource localAccount(m_accounts.value(path).account());

    // Change the relationships based on the PresenceState value.
    if (state == Tp::Contact::PresenceStateYes) {
        // Add we publishes to them.
        localAccount.addProperty(Nepomuk::Vocabulary::NCO::publishesPresenceTo(), imAccount.uri());

        // Remove requested from them.
        imAccount.remove(Nepomuk::Vocabulary::NCO::requestedPresenceSubscriptionTo(), localAccount.uri());

    } else if (state ==  Tp::Contact::PresenceStateAsk) {
        // They request subscription to us
        imAccount.addProperty(Nepomuk::Vocabulary::NCO::requestedPresenceSubscriptionTo(), localAccount.uri());

        // Remove them from our publish list.
        localAccount.remove(Nepomuk::Vocabulary::NCO::publishesPresenceTo(), imAccount.uri());

    } else if (state == Tp::Contact::PresenceStateNo) {
        // Remove them from the requested-to-us list
        imAccount.remove(Nepomuk::Vocabulary::NCO::requestedPresenceSubscriptionTo(), localAccount.uri());

        // Remove them from our publish list
        localAccount.remove(Nepomuk::Vocabulary::NCO::publishesPresenceTo(), imAccount.uri());

    } else {
        kWarning() << "Invalid Tp::Contact::PresenceState received.";
        Q_ASSERT(false);
    }

    saveGraph(Nepomuk::SimpleResourceGraph() << imAccount << localAccount);*/
}

void NepomukStorage::setContactCapabilities(const QString &path,
                                           const QString &id,
                                           const Tp::ContactCapabilities &capabilities)
{
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    QHash<ContactIdentifier, ContactResources>::const_iterator it = m_contacts.find(identifier);
    const bool found = (it != m_contacts.constEnd());
    Q_ASSERT(found);
    if (!found) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = it.value();

    Nepomuk::SimpleResource imAccount(resources.imAccount());

    // For each supported (by the ontology) capability, check it and save the correct value
    QVariantList capList;
    if (capabilities.textChats())
        capList << NCO::imCapabilityText();
    if (capabilities.streamedMediaAudioCalls())
        capList << NCO::imCapabilityAudio();
    if (capabilities.streamedMediaVideoCalls())
        capList << NCO::imCapabilityVideo();

    // FIXME: Add other caps to the ontologies so that we can add them here.

    KJob *job = Nepomuk::setProperty( QList<QUrl>() << resources.imAccount(),
                                      NCO::hasIMCapability(),
                                      capList );
    job->exec();
    if( job->error() ) {
        kWarning() << job->errorString();
    }
}

void NepomukStorage::setContactAvatar(const QString &path,
                                      const QString &id,
                                      const Tp::AvatarData &avatar)
{
    ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    QHash<ContactIdentifier, ContactResources>::const_iterator it = m_contacts.find(identifier);
    const bool found = (it != m_contacts.constEnd());
    Q_ASSERT(found);
    if (!found) {
        kWarning() << "Contact not found.";
        return;
    }

    ContactResources resources = it.value();

    if( avatar.fileName.isEmpty() ) {
        kDebug() << "No Avatar set";

        KJob *job = Nepomuk::removeProperties( QList<QUrl>() << resources.imAccount(),
                                               QList<QUrl>() << Telepathy::avatar() );
        job->exec();
        if( job->error() ) {
            kWarning() << job->errorString();
        }
        return;
    }

    //FIXME: Remove the old avatar from the photos list?
    // Otherwise
    Nepomuk::SimpleResource personContact(resources.personContact());
    Nepomuk::SimpleResource imAccount(resources.imAccount());

    QUrl fileUrl( avatar.fileName );
    fileUrl.setScheme(QLatin1String("file"));

    personContact.setProperty( NCO::photo(), fileUrl );
    imAccount.setProperty( Telepathy::avatar(), fileUrl );

    KJob *job = Nepomuk::storeResources( Nepomuk::SimpleResourceGraph() << personContact << imAccount,
                                         Nepomuk::IdentifyNew, Nepomuk::OverwriteProperties );
    job->exec();
    if( job->error() ) {
        kWarning() << job->errorString();
    }

    //TODO: Find a way to index the file as well.
}


int qHash(ContactIdentifier c)
{
    // FIXME: This is a shit way of doing it.
    QString temp;
    temp.reserve( c.accountId().size() + 8 + c.contactId().size() );
    temp.append(c.accountId());
    temp.append(QLatin1String("#--__--#"));
    temp.append(c.contactId());
    return qHash(temp);
}


#include "nepomuk-storage.moc"

