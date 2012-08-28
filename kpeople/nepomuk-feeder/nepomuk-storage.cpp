/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2009-2011 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
 *
 * Copyright (C) 2011-2012 Vishesh Handa <me@vhanda.in>
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
#include "telepathy.h"

#include <KDebug>
#include <KJob>

#include <nepomuk2/datamanagement.h>
#include <nepomuk2/storeresourcesjob.h>
#include <Nepomuk2/Resource>
#include <Nepomuk2/ResourceManager>
#include <nepomuk2/simpleresource.h>
#include <nepomuk2/simpleresourcegraph.h>
#include <Nepomuk2/Variant>

#include <Nepomuk2/Vocabulary/NCO>
#include <Nepomuk2/Vocabulary/PIMO>
#include <Soprano/Vocabulary/RDF>

#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/AndTerm>
#include <Nepomuk2/Query/ComparisonTerm>
#include <Nepomuk2/Query/LiteralTerm>
#include <Nepomuk2/Query/NegationTerm>
#include <Nepomuk2/Query/ResourceTerm>
#include <Nepomuk2/Query/ResourceTypeTerm>
#include <Nepomuk2/Query/QueryServiceClient>
#include <Nepomuk2/Query/Result>

#include <QtCore/QSharedData>
#include <QtCore/QTimer>

#include <TelepathyQt/Constants>
#include <TelepathyQt/AvatarData>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Model>
#include <Soprano/QueryResultIterator>

using namespace Nepomuk2::Vocabulary;
using namespace Soprano::Vocabulary;

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

bool AccountResources::isEmpty() const
{
    return d->account.isEmpty() && d->protocol.isEmpty();
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
    Data(const QUrl &pc,  const QUrl &ia)
      : personContact(pc),
        imAccount(ia)
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

    QUrl personContact;
    QUrl imAccount;
};

ContactResources::ContactResources(const QUrl &personContact,
                                   const QUrl &imAccount)
  : d(new Data(personContact, imAccount))
{ }

ContactResources::ContactResources()
  : d(new Data())
{ }

ContactResources::ContactResources(const ContactResources &other)
  : d(other.d)
{ }

ContactResources::~ContactResources()
{ }

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
    return ((other.personContact() == personContact()) &&
            (other.imAccount() == imAccount()));
}

bool ContactResources::operator!=(const ContactResources& other) const
{
    return !(*this == other);
}

bool ContactResources::isEmpty() const
{
    return d->personContact.isEmpty() && d->imAccount.isEmpty();
}


NepomukStorage::NepomukStorage(QObject *parent)
: AbstractStorage(parent)
{
    QTimer::singleShot(0, this, SLOT(init()));

    m_graphTimer.setSingleShot( true );
    connect( &m_graphTimer, SIGNAL(timeout()), this, SLOT(onContactTimer()) );
}

NepomukStorage::~NepomukStorage()
{
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
    QUrl meUri("nepomuk:/me");

    Soprano::Model* model = Nepomuk2::ResourceManager::instance()->mainModel();
    bool exists = model->containsAnyStatement( meUri, RDF::type(), PIMO::Person() );
    if( !exists ) {
        // It should have been created by the Storage Service
        // TODO: Create it
        kWarning() << "nepomuk:/me doesn't exist!! Aborting";
        return;
    }

    QString query = QString::fromLatin1("select ?o where { <nepomuk:/me> %1 ?o. ?o a %2 .}")
                    .arg( Soprano::Node::resourceToN3( PIMO::groundingOccurrence() ),
                          Soprano::Node::resourceToN3( NCO::PersonContact() ) );

    Soprano::QueryResultIterator it = model->executeQuery( query, Soprano::Query::QueryLanguageSparql );

    QList<QUrl> groundingOccurrences;
    while( it.next() ) {
        groundingOccurrences << it["o"].uri();
    }

    if( groundingOccurrences.isEmpty() ) {
        Nepomuk2::SimpleResource personContact;
        personContact.addType( NCO::PersonContact() );

        Nepomuk2::SimpleResource me( meUri );
        me.addProperty( PIMO::groundingOccurrence(), personContact );

        Nepomuk2::SimpleResourceGraph graph;
        graph << me << personContact;

        Nepomuk2::StoreResourcesJob * job = Nepomuk2::storeResources( graph );
        job->exec();

        if( job->error() ) {
            kWarning() << job->errorString();
            return;
        }
        groundingOccurrences << job->mappings().value( personContact.uri() );
    }

    //FIXME: Use all of them, not just the first one
    m_mePersonContact = groundingOccurrences.first();

    // *********************************************************************************************
    // Load all the relevant accounts and contacts that are already in Nepomuk.

    // We load everything now, because this is much more efficient than re-running a query for each
    // and every account and contact individually when we get to that one.

    // Query Nepomuk for all of the ME PersonContact's IMAccounts.
    {
        QString query = QString::fromLatin1("select distinct ?r ?protocol ?accountIdentifier where {"
                                            " ?r a nco:IMAccount . ?r nco:imAccountType ?protocol ."
                                            " ?r %1 ?accountIdentifier . "
                                            " %2 nco:hasIMAccount ?r . }")
                        .arg( Soprano::Node::resourceToN3(Telepathy::accountIdentifier()),
                              Soprano::Node::resourceToN3( m_mePersonContact ) );

        Soprano::QueryResultIterator it = model->executeQuery( query, Soprano::Query::QueryLanguageSparql );
        while( it.next() ) {
            QUrl imAccount(it["r"].uri());

            // If no Telepathy identifier, then the account is ignored.
            QString identifier = it["accountIdentifier"].literal().toString();
            // FIXME: This doesn't seem possible
            if (identifier.isEmpty()) {
                kDebug() << "Account does not have a Telepathy Account Identifier. Oops. Ignoring.";
                continue;
            }


            // If it does have a telepathy identifier, then it is added to the cache.
            AccountResources acRes( imAccount, it["protocol"].literal().toString() );
            m_accounts.insert( identifier, imAccount );
        }

        QTimer::singleShot( 0, this, SLOT(onAccountsQueryFinishedListing()) );
    }
}


void NepomukStorage::onAccountsQueryFinishedListing()
{
    kDebug() << "Accounts Query Finished Successfully.";
    Soprano::Model* model = Nepomuk2::ResourceManager::instance()->mainModel();
    // Got all the accounts, now move on to the contacts.

    QHashIterator<QString, AccountResources> iter( m_accounts );
    while( iter.hasNext() ) {
        iter.next();

        const QString& accountId = iter.key();
        const AccountResources& accRes = iter.value();

        QString query = QString::fromLatin1("select distinct ?r ?id ?contact where { ?r a nco:IMAccount . "
                                            " ?r nco:imID ?id ; nco:accessedBy %1 . "
                                            " ?contact nco:hasIMAccount ?r . }")
                        .arg( Soprano::Node::resourceToN3(accRes.account()) );

        Soprano::QueryResultIterator it = model->executeQuery( query, Soprano::Query::QueryLanguageSparql );
        while( it.next() ) {

            QUrl imAccount = it["r"].uri();
            QUrl contact = it["contact"].uri();
            QString id = it["id"].literal().toString();

            ContactIdentifier ci( accountId, id );
            ContactResources cr( contact, imAccount );

            m_contacts.insert( ci, cr );
        }
    }

    emit initialised( true );
}


void NepomukStorage::cleanupAccounts(const QList<QString> &paths)
{
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
    if( !removedAccounts.isEmpty() ) {
        KJob *job = Nepomuk2::removeResources( removedAccounts, Nepomuk2::RemoveSubResoures );
        job->exec();
        if( job->error() ) {
            kWarning() << job->errorString();
        }
    }

    // Go through all the accounts that we have received from the controller and create any
    // new ones in neponmuk. Do this as a batch job to improve performance.
    /*foreach (const QString &path, paths) {
        if (!m_accounts.keys().contains(path)) {
            // TODO: Implement me to do this as a batch job???
            //       For now, we just let the constructed signal do this one at a time.
        }
    }*/
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

    Nepomuk2::SimpleResource imAccount;
    imAccount.addType(Nepomuk2::Vocabulary::NCO::IMAccount());
    imAccount.addProperty(Nepomuk2::Vocabulary::Telepathy::accountIdentifier(), path);
    imAccount.addProperty(Nepomuk2::Vocabulary::NCO::imAccountType(), protocol);
    imAccount.addProperty(Nepomuk2::Vocabulary::NCO::imID(), id);

    Nepomuk2::SimpleResource mePersonContact(m_mePersonContact);
    mePersonContact.addProperty(Nepomuk2::Vocabulary::NCO::hasIMAccount(), imAccount);

    Nepomuk2::SimpleResourceGraph graph;
    graph << imAccount << mePersonContact;

    Nepomuk2::StoreResourcesJob* job = graph.save();
    job->exec();
    if( job->error() ) {
        kError() << job->error();
        Q_ASSERT( job->error() );
    }

    const QUrl imAccountUri = job->mappings().value( imAccount.uri() );
    m_accounts.insert(path, AccountResources(imAccountUri, protocol));
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

AccountResources NepomukStorage::findAccount(const QString& path)
{
    QHash<QString, AccountResources>::const_iterator it = m_accounts.constFind(path);
    if( it == m_accounts.constEnd() ) {
        kWarning() << "Account not found: " << path;
        return AccountResources();
    }

    return it.value();
}

void NepomukStorage::setAccountNickname(const QString &path, const QString &nickname)
{
    AccountResources account = findAccount(path);
    if( account.isEmpty() )
        return;

    QUrl accountUri = account.account();

    // imNickName does not have a max cardinality of 1
    // so we cannot use to storeResources, as it will just add another value
    KJob* job = Nepomuk2::setProperty( QList<QUrl>() << accountUri, NCO::imNickname(),
                                      QVariantList() << nickname );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(onSaveJobResult(KJob*)) );
}

void NepomukStorage::setAccountCurrentPresence(const QString &path, const Tp::SimplePresence &presence)
{
    AccountResources account = findAccount(path);
    if( account.isEmpty() )
        return;

    QUrl accountUri = account.account();

    Nepomuk2::SimpleResource &accountRes = m_graph[accountUri];
    accountRes.setUri( accountUri );
    accountRes.setProperty(NCO::imStatus(), presence.status);
    accountRes.setProperty(NCO::imStatusMessage(), presence.statusMessage);
    accountRes.setProperty(Telepathy::statusType(), presence.type);

    fireGraphTimer();
}

//
// Contact Functions
//

ContactResources NepomukStorage::findContact(const QString& path, const QString& id)
{
    const ContactIdentifier identifier(path, id);

    // Check the Contact exists.
    QHash<ContactIdentifier, ContactResources>::const_iterator it = m_contacts.find(identifier);
    const bool found = (it != m_contacts.constEnd());
    if (!found) {
        kWarning() << "Contact not found:" << path << id;
        return ContactResources();
    }

    return it.value();
}

void NepomukStorage::fireGraphTimer()
{
    if( !m_graphTimer.isActive() )
        m_graphTimer.start( 500 );
}



void NepomukStorage::cleanupAccountContacts(const QString &path, const QList<QString> &ids)
{
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
    // new ones in Nepomuk.
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
    // First, check that we don't already have a record for this contact.
    ContactIdentifier identifier(path, id);
    if (m_contacts.contains(identifier)) {
//        kDebug() << "Contact record already exists. Return.";
        return;
    }

    // Contact not found. Need to create it.
    kDebug() << path << id;
    kDebug() << "Contact not found in Nepomuk. Creating it.";

    Q_ASSERT(m_accounts.contains(path));
    if (!m_accounts.contains(path)) {
        kWarning() << "Corresponding account not cached.";
        return;
    }

    AccountResources accountRes = m_accounts.value(path);
    QUrl accountUri = accountRes.account();

    Nepomuk2::SimpleResource newPersonContact;
    newPersonContact.addType(Nepomuk2::Vocabulary::NCO::PersonContact());

    Nepomuk2::SimpleResource newImAccount;
    //TODO: Somehow add this imAccount as a sub resource the account, maybe.
    newImAccount.addType(Nepomuk2::Vocabulary::NCO::IMAccount());
    newImAccount.setProperty(Nepomuk2::Vocabulary::NCO::imStatus(), QString::fromLatin1("unknown"));
    newImAccount.setProperty(Nepomuk2::Vocabulary::NCO::imID(), id);
    newImAccount.setProperty(Nepomuk2::Vocabulary::Telepathy::statusType(), Tp::ConnectionPresenceTypeUnknown);
    newImAccount.setProperty(Nepomuk2::Vocabulary::NCO::imAccountType(), accountRes.protocol());
    newImAccount.addProperty(Nepomuk2::Vocabulary::NCO::isAccessedBy(), accountUri);

    newPersonContact.addProperty(Nepomuk2::Vocabulary::NCO::hasIMAccount(), newImAccount);

    Nepomuk2::SimpleResourceGraph graph;
    graph << newPersonContact << newImAccount;

    Nepomuk2::StoreResourcesJob* job = graph.save();
    job->exec();
    if( job->error() ) {
        kError() << job->errorString();
        Q_ASSERT( job->error() );
        return;
    }

    const QUrl personContactUri = job->mappings().value( newPersonContact.uri() );
    const QUrl imAccountUri = job->mappings().value( newImAccount.uri() );
    m_contacts.insert( identifier, ContactResources(personContactUri, imAccountUri) );
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
    ContactResources contact = findContact(path, id);
    if( contact.isEmpty() )
        return;

    QUrl imAccountUri = contact.imAccount();

    // imNickName does not have a max cardinality of 1
    // so we cannot use to storeResources, as it will just add another value
    KJob* job = Nepomuk2::setProperty( QList<QUrl>() << imAccountUri, NCO::imNickname(),
                                      QVariantList() << alias );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(onSaveJobResult(KJob*)) );
}

void NepomukStorage::setContactPresence(const QString &path,
                                 const QString &id,
                                 const Tp::SimplePresence &presence)
{
    ContactResources contact = findContact(path, id);
    if( contact.isEmpty() )
        return;

    QUrl imAccountUri = contact.imAccount();

    hasInvalidResources();
    Nepomuk2::SimpleResource &imAccount = m_graph[imAccountUri];
    imAccount.setUri( imAccountUri );
    imAccount.setProperty(NCO::imStatus(), presence.status);
    imAccount.setProperty(NCO::imStatusMessage(), presence.statusMessage);
    imAccount.setProperty(Telepathy::statusType(), presence.type);

    hasInvalidResources();
    fireGraphTimer();
}

void NepomukStorage::setContactGroups(const QString &path,
                                      const QString &id,
                                      const QStringList &groups)
{
    //kDebug() << path << id << groups;
    ContactResources contact = findContact(path, id);
    if( contact.isEmpty() )
        return;

    if( groups.isEmpty() ) {
        KJob* job = Nepomuk2::removeProperties( QList<QUrl>() << contact.personContact(),
                                               QList<QUrl>() << NCO::belongsToGroup() );
        connect( job, SIGNAL(finished(KJob*)), this, SLOT(onRemovePropertiesJob(KJob*)) );
        //TODO: Maybe remove empty groups?
        return;
    }

    //FIXME: Ideally cache all the group uris
    QVariantList groupUris;
    foreach( const QString& groupName, groups ) {
        Nepomuk2::SimpleResource groupRes;
        groupRes.addType( NCO::ContactGroup() );
        groupRes.setProperty( NCO::contactGroupName(), groupName );

        groupUris << groupRes.uri();
        m_graph << groupRes;
    }

    QUrl contactUri = contact.personContact();

    Nepomuk2::SimpleResource &contactRes = m_graph[contactUri];
    contactRes.setUri( contactUri );
    contactRes.setProperty( NCO::belongsToGroup(), groupUris );

    fireGraphTimer();
}

void NepomukStorage::setContactBlockStatus(const QString &path, const QString &id, bool blocked)
{
    ContactResources contact = findContact(path, id);
    if( contact.isEmpty() )
        return;

    QUrl imAccountUri = contact.imAccount();

    hasInvalidResources();
    Nepomuk2::SimpleResource &imAccount = m_graph[imAccountUri];
    imAccount.setUri( imAccountUri );
    imAccount.setProperty( NCO::isBlocked(), blocked );

    hasInvalidResources();
    fireGraphTimer();
}

void NepomukStorage::setContactPublishState(const QString &path,
                                     const QString &id,
                                     const Tp::Contact::PresenceState &state)
{
    ContactResources contact = findContact(path, id);
    if( contact.isEmpty() )
        return;

    AccountResources account = m_accounts.value(path);
    if( account.isEmpty() )
        return;

    QUrl imAccountUri = contact.imAccount();
    QUrl localAccountUri = account.account();

    bool usRequest = false;
    bool themPublish = false;

    // Change the relationships based on the PresenceState value.
    switch( state ) {
        case Tp::Contact::PresenceStateYes:
            themPublish = true;
            break;

        case Tp::Contact::PresenceStateAsk:
            usRequest = true;
            break;

        case Tp::Contact::PresenceStateNo:
            break;

        default:
            kWarning() << "Invalid Tp::Contact::PresenceState received.";
            Q_ASSERT(false);
    }

    if( usRequest ) {
        Nepomuk2::SimpleResource &localAccountRes = m_graph[localAccountUri];
        localAccountRes.setUri( localAccountUri );
        localAccountRes.setProperty( NCO::requestedPresenceSubscriptionTo(), imAccountUri );

        fireGraphTimer();
    }
    else {
        // Remove us from the requested-to-them list
        KJob* job = Nepomuk2::removeProperty( QList<QUrl>() << localAccountUri,
                                             NCO::requestedPresenceSubscriptionTo(),
                                             QVariantList() << imAccountUri );
        connect( job, SIGNAL(finished(KJob*)), this, SLOT(onRemovePropertiesJob(KJob*)) );
    }

    if( themPublish ) {
        Nepomuk2::SimpleResource &imAccount = m_graph[imAccountUri];
        imAccount.setUri( imAccountUri );
        imAccount.setProperty( NCO::publishesPresenceTo(), localAccountUri );

        fireGraphTimer();
    }
    else {
        // Remove us from their publish list
        KJob* job = Nepomuk2::removeProperty( QList<QUrl>() << imAccountUri,
                                             NCO::publishesPresenceTo(),
                                             QVariantList() << localAccountUri );
        connect( job, SIGNAL(finished(KJob*)), this, SLOT(onRemovePropertiesJob(KJob*)) );
    }
}

void NepomukStorage::setContactSubscriptionState(const QString &path,
                                          const QString &id,
                                          const Tp::Contact::PresenceState &state)
{
    ContactResources contact = findContact(path, id);
    if( contact.isEmpty() )
        return;

    AccountResources account = m_accounts.value(path);
    if( account.isEmpty() )
        return;

    QUrl imAccountUri = contact.imAccount();
    QUrl localAccountUri = account.account();

    bool usPublish = false;
    bool themRequest = false;

    // Change the relationships based on the PresenceState value.
    switch( state ) {
        case Tp::Contact::PresenceStateYes:
            usPublish = true;
            break;

        case Tp::Contact::PresenceStateAsk:
            themRequest = true;
            break;

        case Tp::Contact::PresenceStateNo:
            break;

        default:
            kWarning() << "Invalid Tp::Contact::PresenceState received.";
            Q_ASSERT(false);
    }

    if( usPublish ) {
        Nepomuk2::SimpleResource &localAccountRes = m_graph[localAccountUri];
        localAccountRes.setUri( localAccountUri );
        localAccountRes.setProperty( NCO::publishesPresenceTo(), imAccountUri );

        fireGraphTimer();
    }
    else {
        // Remove us from the requested-to-them list
        KJob* job = Nepomuk2::removeProperty( QList<QUrl>() << localAccountUri,
                                             NCO::publishesPresenceTo(),
                                             QVariantList() << imAccountUri );
        connect( job, SIGNAL(finished(KJob*)), this, SLOT(onRemovePropertiesJob(KJob*)) );
    }

    if( themRequest ) {
        Nepomuk2::SimpleResource &imAccount = m_graph[imAccountUri];
        imAccount.setUri( imAccountUri );
        imAccount.setProperty( NCO::requestedPresenceSubscriptionTo(), localAccountUri );

        fireGraphTimer();
    }
    else {
        // Remove us from their publish list
        KJob* job = Nepomuk2::removeProperties( QList<QUrl>() << imAccountUri,
                                               QList<QUrl>() << NCO::requestedPresenceSubscriptionTo()
                                                             << NCO::publishesPresenceTo() );
        connect( job, SIGNAL(finished(KJob*)), this, SLOT(onRemovePropertiesJob(KJob*)) );
    }
}

void NepomukStorage::setContactCapabilities(const QString &path,
                                           const QString &id,
                                           const Tp::ContactCapabilities &capabilities)
{
    ContactResources contact = findContact(path, id);
    if( contact.isEmpty() )
        return;

    QUrl imAccountUri = contact.imAccount();

    // For each supported (by the ontology) capability, check it and save the correct value
    // FIXME: Add other caps to the ontologies so that we can add them here.
    QVariantList capList;
    if (capabilities.textChats())
        capList << NCO::imCapabilityText();
    if (capabilities.streamedMediaAudioCalls())
        capList << NCO::imCapabilityAudio();
    if (capabilities.streamedMediaVideoCalls())
        capList << NCO::imCapabilityVideo();

    // Cannot use storeResources as nco:hasIMCapability doesn't have a cardinality of 1
    KJob* job = Nepomuk2::setProperty(QList<QUrl>() << imAccountUri, NCO::hasIMCapability(), capList );
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(onSaveJobResult(KJob*)));
}

void NepomukStorage::setContactAvatar(const QString &path,
                                      const QString &id,
                                      const Tp::AvatarData &avatar)
{
    ContactResources contact = findContact(path, id);
    if( contact.isEmpty() )
        return;

    if( avatar.fileName.isEmpty() ) {
        //kDebug() << "No Avatar set";

        KJob *job = Nepomuk2::removeProperties( QList<QUrl>() << contact.imAccount(),
                                               QList<QUrl>() << Telepathy::avatar() );
        job->exec();
        if( job->error() ) {
            kWarning() << job->errorString();
        }
        return;
    }

    //FIXME: Remove the old avatar from the photos list?
    QUrl fileUrl( avatar.fileName );
    fileUrl.setScheme(QLatin1String("file"));

    hasInvalidResources();
    Nepomuk2::SimpleResource& personContact = m_graph[contact.personContact()];
    personContact.setUri(contact.personContact());
    personContact.setProperty( NCO::photo(), fileUrl );

    hasInvalidResources();
    Nepomuk2::SimpleResource& imAccount = m_graph[contact.imAccount()];
    imAccount.setUri(contact.imAccount());
    imAccount.setProperty( Telepathy::avatar(), fileUrl );

    hasInvalidResources();
    fireGraphTimer();
    //TODO: Find a way to index the file as well.
}

void NepomukStorage::onContactTimer()
{
//    kDebug() << m_contactGraph;
    hasInvalidResources();
    kDebug() << m_graph;
    KJob *job = Nepomuk2::storeResources( m_graph, Nepomuk2::IdentifyNew, Nepomuk2::OverwriteProperties );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(onContactGraphJob(KJob*)) );

    m_graph.clear();
}

void NepomukStorage::onContactGraphJob(KJob* job)
{
    if( job->error() ) {
        kWarning() << job->errorString();
        return;
    }

    emit graphSaved();
}

void NepomukStorage::onRemovePropertiesJob(KJob* job)
{
    if( job->error() ) {
        kWarning() << job->errorString();
        return;
    }
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

bool NepomukStorage::hasInvalidResources() const
{
    /*QList<Nepomuk2::SimpleResource> list = m_contactGraph.toList();
    foreach( const Nepomuk2::SimpleResource& res, list ) {
        if( !res.isValid() ) {
            kWarning() << "Found invalid resource";
            Q_ASSERT( false );
            return true;
        }
        if( res.contains( Telepathy::statusType() ) ) {
            QVariantList list = res.property( Telepathy::statusType() );
            if( list.size() > 1 ) {
                kWarning() << "oh noez!!";
                Q_ASSERT( false );
            }
        }
    }*/
    return false;
}


#include "nepomuk-storage.moc"

