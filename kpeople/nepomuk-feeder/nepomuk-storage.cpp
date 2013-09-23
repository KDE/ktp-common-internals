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
#include "capabilities-hack-private.h"

#include <KDebug>
#include <KJob>

#include <Nepomuk2/Resource>
#include <Nepomuk2/ResourceManager>
#include <Nepomuk2/DataManagement>
#include <Nepomuk2/StoreResourcesJob>
#include <Nepomuk2/SimpleResource>
#include <Nepomuk2/SimpleResourceGraph>
#include <Nepomuk2/Variant>

#include <Nepomuk2/Vocabulary/NCO>
#include <Nepomuk2/Vocabulary/PIMO>
#include <Nepomuk2/Vocabulary/NIE>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/NRL>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>

#include <QtCore/QSharedData>
#include <QtCore/QTimer>

#include <TelepathyQt/Constants>
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/Connection>

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

AccountResources::AccountResources(const QUrl &account, const QString &protocol)
    : d(new Data(account, protocol))
{
}

AccountResources::AccountResources()
    : d(new Data())
{
}

AccountResources::AccountResources(const AccountResources &other)
    : d(other.d)
{
}

AccountResources::AccountResources(const QUrl &url)
    : d(new Data(url, QString()))
{
}

AccountResources::~AccountResources()
{
}

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
{
}

ContactIdentifier::ContactIdentifier(const ContactIdentifier &other)
    : d(other.d)
{
}

ContactIdentifier::ContactIdentifier()
    : d(new Data())
{
}

ContactIdentifier::~ContactIdentifier()
{
}

const QString& ContactIdentifier::accountId() const
{
    return d->accountId;
}

const QString& ContactIdentifier::contactId() const
{
    return d->contactId;
}

bool ContactIdentifier::operator==(const ContactIdentifier &other) const
{
    return ((other.accountId() == accountId()) && (other.contactId() == contactId()));
}

bool ContactIdentifier::operator!=(const ContactIdentifier &other) const
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

ContactResources::ContactResources(const QUrl &personContact, const QUrl &imAccount)
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

const QUrl& ContactResources::personContact() const
{
    return d->personContact;
}

const QUrl& ContactResources::imAccount() const
{
    return d->imAccount;
}

bool ContactResources::operator==(const ContactResources &other) const
{
    return ((other.personContact() == personContact()) &&
            (other.imAccount() == imAccount()));
}

bool ContactResources::operator!=(const ContactResources &other) const
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

    m_graphTimer.setSingleShot(true);
    connect(&m_graphTimer, SIGNAL(timeout()), this, SLOT(onContactTimer()));
}

NepomukStorage::~NepomukStorage()
{
}

void NepomukStorage::init()
{
    // *********************************************************************************************
    // Get the ME nco:PersonContact (creating them if necessary)

    QLatin1String query("select ?o where { <nepomuk:/me> pimo:groundingOccurrence ?o."
                        "?o a nco:PersonContact .}");
    Soprano::Model *model = Nepomuk2::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    QList<QUrl> groundingOccurrences;
    while (it.next()) {
        groundingOccurrences << it["o"].uri();
    }

    if (groundingOccurrences.isEmpty()) {
        Nepomuk2::SimpleResource personContact;
        personContact.addType(NCO::PersonContact());

        Nepomuk2::SimpleResource me(QUrl("nepomuk:/me"));
        me.addProperty(PIMO::groundingOccurrence(), personContact);

        Nepomuk2::SimpleResourceGraph graph;
        graph << me << personContact;

        Nepomuk2::StoreResourcesJob *job = Nepomuk2::storeResources(graph);
        job->exec();

        if (job->error()) {
            kWarning() << job->errorString();
            return;
        }
        groundingOccurrences << job->mappings().value(personContact.uri());
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
                        .arg(Soprano::Node::resourceToN3(Telepathy::accountIdentifier()),
                             Soprano::Node::resourceToN3(m_mePersonContact));

        Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);
        while (it.next()) {
            QUrl imAccount(it["r"].uri());

            // If no Telepathy identifier, then the account is ignored.
            QString identifier = it["accountIdentifier"].literal().toString();
            // FIXME: This doesn't seem possible
            if (identifier.isEmpty()) {
                kDebug() << "Account does not have a Telepathy Account Identifier. Oops. Ignoring.";
                continue;
            }


            // If it does have a telepathy identifier, then it is added to the cache.
            AccountResources acRes(imAccount, it["protocol"].literal().toString());
            m_accounts.insert(identifier, acRes);
        }

        QTimer::singleShot(0, this, SLOT(onAccountsQueryFinishedListing()));
    }
}


void NepomukStorage::onAccountsQueryFinishedListing()
{
    kDebug() << "Accounts Query Finished Successfully.";
    Soprano::Model *model = Nepomuk2::ResourceManager::instance()->mainModel();
    // Got all the accounts, now move on to the contacts.

    QHashIterator<QString, AccountResources> iter(m_accounts);
    while (iter.hasNext()) {
        iter.next();

        const QString &accountId = iter.key();
        const AccountResources &accRes = iter.value();

        QString query = QString::fromLatin1("select distinct ?r ?id ?contact where { ?r a nco:IMAccount . "
                                            " ?r nco:imID ?id ; nco:isAccessedBy %1 . "
                                            " ?contact nco:hasIMAccount ?r . }")
                        .arg(Soprano::Node::resourceToN3(accRes.account()));

        Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);
        while (it.next()) {

            QUrl imAccount = it["r"].uri();
            QUrl contact = it["contact"].uri();
            QString id = it["id"].literal().toString();

            ContactIdentifier ci(accountId, id);
            ContactResources cr(contact, imAccount);

            m_contacts.insert(ci, cr);
        }
    }

    Q_EMIT initialised(true);
}


void NepomukStorage::cleanupAccounts(const QList<QString> &paths)
{
    QSet<QString> pathSet = paths.toSet();

    // Go through all our accounts and remove all the ones that are not there in the path
    // list given by the controller
    QList<QUrl> removedAccounts;
    QMutableHashIterator<QString, AccountResources> it(m_accounts);
    while (it.hasNext()) {
        it.next();
        if (!pathSet.contains(it.key())) {
            removedAccounts << it.value().account();
            it.remove();
        }
    }

    // TODO: Do this properly once the ontology supports this
    // TODO: What do we do with an account in nepomuk which the use has removed?
    //       For now we're just deleting them
    if (!removedAccounts.isEmpty()) {
        KJob *job = Nepomuk2::removeResources(removedAccounts, Nepomuk2::RemoveSubResoures);
        job->exec();
        if (job->error()) {
            kWarning() << job->errorString();
        }
    }

    // Go through all the accounts that we have received from the controller and create any
    // new ones in neponmuk. Do this as a batch job to improve performance.
    /*Q_FOREACH (const QString &path, paths) {
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
    imAccount.addType(NCO::IMAccount());
    imAccount.addProperty(Telepathy::accountIdentifier(), path);
    imAccount.addProperty(NCO::imAccountType(), protocol);
    imAccount.addProperty(NCO::imID(), id);

    Nepomuk2::SimpleResource mePersonContact(m_mePersonContact);
    mePersonContact.addProperty(NCO::hasIMAccount(), imAccount);

    Nepomuk2::SimpleResourceGraph graph;
    graph << imAccount << mePersonContact;

    Nepomuk2::StoreResourcesJob *job = graph.save();
    job->exec();
    if (job->error()) {
        kError() << job->error();
        Q_ASSERT(job->error());
    }

    const QUrl imAccountUri = job->mappings().value(imAccount.uri());
    m_accounts.insert(path, AccountResources(imAccountUri, protocol));
}

AccountResources NepomukStorage::findAccount(const QString &path)
{
    QHash<QString, AccountResources>::const_iterator it = m_accounts.constFind(path);
    if (it == m_accounts.constEnd()) {
        kWarning() << "Account not found: " << path;
        return AccountResources();
    }

    return it.value();
}

void NepomukStorage::setAccountNickname(const QString &path, const QString &nickname)
{
    AccountResources account = findAccount(path);
    if (account.isEmpty()) {
        return;
    }

    Nepomuk2::SimpleResource &accountRes = m_graph[account.account()];
    accountRes.setProperty(NCO::imNickname(), nickname);

    fireGraphTimer();
}

//
// Contact Functions
//

ContactResources NepomukStorage::findContact(const QString &path, const QString &id)
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
    if (!m_graphTimer.isActive()) {
        m_graphTimer.start(500);
    }
}



void NepomukStorage::cleanupAccountContacts(const QString &path, const Tp::Contacts &contacts)
{
    QHash<QString, Tp::ContactPtr> contactHash;

    Q_FOREACH (const Tp::ContactPtr &contact, contacts) {
        contactHash.insert(contact->id(), contact);
    }

    // Go through all the contacts that we have received from the account and create any
    // new ones in Nepomuk.
    QSet<QString> nepomukIds;
    Q_FOREACH (const ContactIdentifier &ci, m_contacts.keys()) {
        nepomukIds.insert(ci.contactId());
    }

    QSet<QString> oldIds = contactHash.keys().toSet();

    QSet<QString> newIds = oldIds.subtract(nepomukIds);
    Q_FOREACH (const QString &id, newIds) {
        createContact(path, contactHash[id]);
    }
}

void NepomukStorage::onAccountRemoved(const QString &path)
{
    AccountResources accountRes = m_accounts.value(path);
    QUrl accountUri = accountRes.account();

    QList<QUrl> urisToDelete;
    urisToDelete << accountUri;

    QString query = QString::fromLatin1("select DISTINCT ?uri ?a "
                                        "WHERE { "
                                        "?uri a nco:PersonContact. "
                                        "?uri nco:hasIMAccount ?a . "
                                        "?a nco:isAccessedBy %1 . }").arg(Soprano::Node::resourceToN3(accountUri));

    Soprano::Model *model = Nepomuk2::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    while (it.next()) {
        urisToDelete << it["uri"].uri() << it["a"].uri();
    }

    KJob *removeJob = Nepomuk2::removeDataByApplication(urisToDelete);
    connect(removeJob, SIGNAL(finished(KJob*)),
            this, SLOT(onContactGraphJob(KJob*)));
}

void NepomukStorage::removeContact(const QString& path, const Tp::ContactPtr& contact)
{
    ContactIdentifier identifier(path, contact->id());
    if (!m_contacts.contains(identifier)) {
        return;
    }
    QUrl contactUri = m_contacts[identifier].personContact();
    Nepomuk2::removeResources(QList<QUrl>() << contactUri);
}


void NepomukStorage::createContact(const QString &path, const Tp::ContactPtr &contact)
{
    if (contact.isNull()) {
        return;
    }
    // First, check that we don't already have a record for this contact.
    ContactIdentifier identifier(path, contact->id());
    if (m_contacts.contains(identifier)) {
//        kDebug() << "Contact record already exists. Return.";
        return;
    }

    // Contact not found. Need to create it.
    kDebug() << path << contact->id();
    kDebug() << "Contact not found in Nepomuk. Creating it.";

    Q_ASSERT(m_accounts.contains(path));
    if (!m_accounts.contains(path)) {
        kWarning() << "Corresponding account not cached.";
        return;
    }

    AccountResources accountRes = m_accounts.value(path);
    QUrl accountUri = accountRes.account();

    Nepomuk2::SimpleResource newPersonContact;
    newPersonContact.addType(NCO::PersonContact());
    newPersonContact.addType(NIE::DataObject());

    Nepomuk2::SimpleResource newImAccount;
    //TODO: Somehow add this imAccount as a sub resource the account, maybe.
    newImAccount.addType(NCO::IMAccount());
    newImAccount.setProperty(NCO::imID(), contact->id());
    newImAccount.setProperty(NCO::imAccountType(), accountRes.protocol());
    newImAccount.addProperty(NCO::isAccessedBy(), accountUri);

    newPersonContact.setProperty(NCO::contactUID(), path + QLatin1Char('_') + contact->id());

    newPersonContact.addProperty(NCO::hasIMAccount(), newImAccount);
    updateAlias(newPersonContact, newImAccount, contact->alias());
    updateContactGroups(newPersonContact, contact->groups());
    updateContactAvatar(newPersonContact, newImAccount, contact->avatarData());


    Nepomuk2::SimpleResourceGraph graph;
    graph << newPersonContact << newImAccount;

    QHash<QUrl, QVariant> additional;
    additional.insert(RDF::type(), NRL::DiscardableInstanceBase());

    Nepomuk2::StoreResourcesJob *job = Nepomuk2::storeResources(graph, Nepomuk2::IdentifyNone,
                                                                Nepomuk2::NoStoreResourcesFlags,
                                                                additional);
    job->exec();
    if (job->error()) {
        kError() << job->errorString();
        Q_ASSERT(job->error());
        return;
    }

    const QUrl personContactUri = job->mappings().value(newPersonContact.uri());
    const QUrl imAccountUri = job->mappings().value(newImAccount.uri());
    m_contacts.insert(identifier, ContactResources(personContactUri, imAccountUri));
}

void NepomukStorage::setContactAlias(const QString &path, const QString &id, const QString &alias)
{
    ContactResources contact = findContact(path, id);
    if (contact.isEmpty()) {
        return;
    }

    Nepomuk2::SimpleResource &personContact = m_graph[contact.personContact()];
    Nepomuk2::SimpleResource &imAccount = m_graph[contact.imAccount()];

    updateAlias(personContact, imAccount, alias);
    fireGraphTimer();
}

void NepomukStorage::updateAlias(Nepomuk2::SimpleResource &contactResource, Nepomuk2::SimpleResource &imAccountResource, const QString &alias)
{
    contactResource.setProperty(NCO::nickname(), alias);
    imAccountResource.setProperty(NCO::imNickname(), alias);
}


QUrl NepomukStorage::findGroup(const QString& groupName)
{
    QHash< QString, QUrl >::const_iterator fit = m_groupCache.constFind( groupName );
    if (fit != m_groupCache.constEnd()) {
        return fit.value();
    }

    const QString query = QString::fromLatin1("select ?g where { ?g nco:contactGroupName %1 . } LIMIT 1")
                          .arg(Soprano::Node::literalToN3(groupName));

    Soprano::Model* model = Nepomuk2::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator iter = model->executeQuery( query,
                                                Soprano::Query::QueryLanguageSparqlNoInference );

    if (iter.next()) {
        const QUrl uri = iter[0].uri();
        m_groupCache.insert(groupName, uri);
        return uri;
    }

    Nepomuk2::SimpleResource groupRes;
    groupRes.addType(NCO::ContactGroup());
    groupRes.setProperty(NCO::contactGroupName(), groupName);

    Nepomuk2::SimpleResourceGraph graph;
    graph << groupRes;

    QHash<QUrl, QVariant> additional;
    additional.insert(RDF::type(), NRL::DiscardableInstanceBase());

    Nepomuk2::StoreResourcesJob *job = Nepomuk2::storeResources(graph, Nepomuk2::IdentifyNew,
                                                                Nepomuk2::NoStoreResourcesFlags,
                                                                additional);
    // This is blocking, but the code is much easier this way
    job->exec();

    const QUrl uri = job->mappings().value(groupRes.uri());
    m_groupCache.insert(groupName, uri);
    return uri;
}


void NepomukStorage::setContactGroups(const QString &path,
                                      const QString &id,
                                      const QStringList &groups)
{
    //kDebug() << path << id << groups;
    ContactResources contact = findContact(path, id);
    if (contact.isEmpty()) {
        return;
    }

    if (groups.isEmpty()) {
        KJob *job = Nepomuk2::removeProperties(QList<QUrl>() << contact.personContact(),
                                               QList<QUrl>() << NCO::belongsToGroup());

        connect(job, SIGNAL(finished(KJob*)), this, SLOT(onContactGraphJob(KJob*)));
        //TODO: Maybe remove empty groups?
        return;
    }

    Nepomuk2::SimpleResource &personContact = m_graph[contact.personContact()];

    updateContactGroups(personContact, groups);
    fireGraphTimer();
}

void NepomukStorage::updateContactGroups(Nepomuk2::SimpleResource &contactResource, const QStringList &groups)
{
    QVariantList groupUris;
    Q_FOREACH (const QString &groupName, groups) {
        groupUris << findGroup(groupName);
    }
    contactResource.setProperty(NCO::belongsToGroup(), groupUris);
}



void NepomukStorage::setContactAvatar(const QString &path,
                                      const QString &id,
                                      const Tp::AvatarData &avatar)
{
    ContactResources contact = findContact(path, id);
    if (contact.isEmpty()) {
        return;
    }


    //FIXME: Do not remove the old avatar from the photos list?
    Nepomuk2::SimpleResource &personContact = m_graph[contact.personContact()];
    Nepomuk2::SimpleResource &imAccount = m_graph[contact.imAccount()];
    updateContactAvatar(personContact, imAccount, avatar);

    fireGraphTimer();
    //TODO: Find a way to index the file as well.
}

void NepomukStorage::updateContactAvatar(Nepomuk2::SimpleResource &contactResource, Nepomuk2::SimpleResource &imAccountResource, const Tp::AvatarData &avatar)
{
    const QUrl &avatarUrl = avatar.fileName;
    if (avatarUrl.isEmpty()) {
        return;
    }

    contactResource.setProperty(NCO::photo(), avatarUrl);
    imAccountResource.setProperty(Telepathy::avatar(), avatarUrl);
}

void NepomukStorage::onContactTimer()
{
    QHash<QUrl, QVariant> additional;
    additional.insert(RDF::type(), NRL::DiscardableInstanceBase());

    KJob *job = Nepomuk2::storeResources(m_graph, Nepomuk2::IdentifyNew,
                                         Nepomuk2::OverwriteAllProperties, additional);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(onContactGraphJob(KJob*)));

    m_graph.clear();
}

void NepomukStorage::onContactGraphJob(KJob *job)
{
    if (job->error()) {
        kError() << job->errorString();
    }
}


uint qHash(const ContactIdentifier& c)
{
    // FIXME: This is a shit way of doing it.
    QString temp;
    temp.reserve(c.accountId().size() + 8 + c.contactId().size());
    temp.append(c.accountId());
    temp.append(QLatin1String("#--__--#"));
    temp.append(c.contactId());
    return qHash(temp);
}



#include "nepomuk-storage.moc"

