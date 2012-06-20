/*
 * This file is part of nepomuktelepathyservice
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @author Dario Freddi <dario.freddi@collabora.co.uk>
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

#include "contact-test.h"
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/ContactManager>
#include <Soprano/Node>
#include "ontologies/telepathy.h"
#include "ontologies/nco.h"
#include <Nepomuk2/ResourceManager>
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include "ontologies/imaccount.h"
#include <TelepathyQt4/Account>
#include <TelepathyQt4/PendingVoid>

ContactTest::ContactTest(QObject* parent)
    : TidBaseTest(parent)
{

}

ContactTest::~ContactTest()
{

}
void ContactTest::expectPendingContactsFinished(Tp::PendingOperation *op)
{
    if (!op->isFinished()) {
        qWarning() << "unfinished";
        mLoop->exit(1);
        return;
    }

    if (op->isError()) {
        qWarning().nospace() << op->errorName()
            << ": " << op->errorMessage();
        mLoop->exit(2);
        return;
    }

    if (!op->isValid()) {
        qWarning() << "inconsistent results";
        mLoop->exit(3);
        return;
    }

    qDebug() << "finished";
    Tp::PendingContacts *pending = qobject_cast< Tp::PendingContacts* >(op);
    m_contacts = pending->contacts();

    mLoop->exit(0);
}

void ContactTest::initTestCase()
{
    initTestCaseConnectionImpl();

    Tp::Features features = Tp::Features() << Tp::Connection::FeatureRoster;
    QVERIFY(connect(connection()->becomeReady(features),
            SIGNAL(finished(Tp::PendingOperation*)),
            this,
            SLOT(expectSuccessfulCall(Tp::PendingOperation*))));
    QCOMPARE(mLoop->exec(), 0);
    QCOMPARE(connection()->isReady(features), true);
}

void ContactTest::testSetupAccountMonitor()
{
    setupAccountMonitor();
}

void ContactTest::testAccountCreation()
{
    createAccount();

    // Unfortunately there's no other way: let's wait until the account gets created
    QTimer::singleShot(500, mLoop, SLOT(quit()));
    mLoop->exec();

    // Let's wait for the nepomuk resource to become available
    QString query = QString("select distinct ?a where { %1 %2 ?a . ?a a %3 }")
                            .arg(Soprano::Node::resourceToN3(mePersonContact().resourceUri()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::NCO::hasIMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::NCO::IMAccount()));

    Soprano::Model *model = Nepomuk2::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    // Check that we got some results
    QVERIFY(it.next());
    m_accountResource = Nepomuk2::IMAccount(it.binding("a").uri());
    // Check for the validity of the resource
    QVERIFY(m_accountResource.isValid());
    // Check that we got _one_ result
    QVERIFY(!it.next());

    // Go online
    Tp::SimplePresence sp;
    sp.status = "available";
    sp.type = Tp::ConnectionPresenceTypeAvailable;
    sp.statusMessage = QString();
    QVERIFY(connect(account().data()->setRequestedPresence(sp),
                    SIGNAL(finished(Tp::PendingOperation*)),
                    this,
                    SLOT(expectSuccessfulCall(Tp::PendingOperation*))));
    QCOMPARE(mLoop->exec(), 0);
    Tp::PendingVoid *retvoid =
    new Tp::PendingVoid(account()->propertiesInterface()->Set("org.freedesktop.Telepathy.Account", "Connection",
                        QDBusVariant(QVariant::fromValue(connection().data()->objectPath()))), this);
    QVERIFY(connect(retvoid,
                    SIGNAL(finished(Tp::PendingOperation*)),
                    this,
                    SLOT(expectSuccessfulCall(Tp::PendingOperation*))));
    QCOMPARE(mLoop->exec(), 0);
    retvoid =
    new Tp::PendingVoid(account().data()->propertiesInterface()->Set("org.freedesktop.Telepathy.Account",
                                                                     "ConnectionStatus",
                                                                     QDBusVariant(QVariant::fromValue(1))), this);
    QVERIFY(connect(retvoid,
                    SIGNAL(finished(Tp::PendingOperation*)),
                    this,
                    SLOT(expectSuccessfulCall(Tp::PendingOperation*))));
    QCOMPARE(mLoop->exec(), 0);

    qDebug() << account().data()->connection().data();
    QVERIFY(account().data()->haveConnection());
}

void ContactTest::testContactCreation()
{
    // Wait for the contacts to be built
    QStringList ids = QStringList() << QString(QLatin1String("test1@kde.org"))
                                    << QString(QLatin1String("dario.freddi@collabora.co.uk"))
                                    << QString(QLatin1String("george.grundleborg@collabora.co.uk"))
                                    << QString(QLatin1String("drf@kde.org"));
    QVERIFY(connect(connection()->contactManager()->contactsForIdentifiers(ids),
                    SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(expectPendingContactsFinished(Tp::PendingOperation*))));
    QCOMPARE(mLoop->exec(), 0);

    foreach (const Tp::ContactPtr &contact, m_contacts) {
        qDebug() << "Asking subs for" << contact.data()->id();
        QVERIFY(connect(contact.data(),
                        SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
                        mLoop,
                        SLOT(quit())));
        QVERIFY(connect(contact.data(),
                        SIGNAL(publishStateChanged(Tp::Contact::PresenceState)),
                        mLoop,
                        SLOT(quit())));
        contact->requestPresenceSubscription(QLatin1String("please add me"));

        QCOMPARE(mLoop->exec(), 0);
        // I asked to see his presence
        QCOMPARE(static_cast<uint>(contact->subscriptionState()),
                 static_cast<uint>(Tp::Contact::PresenceStateAsk));

        QCOMPARE(mLoop->exec(), 0);
        // He asked to see my presence
        QCOMPARE(static_cast<uint>(contact->publishState()),
                    static_cast<uint>(Tp::Contact::PresenceStateAsk));

        contact->authorizePresencePublication();
        QCOMPARE(mLoop->exec(), 0);
        // I authorized him to see my presence
        QCOMPARE(static_cast<uint>(contact->publishState()),
                    static_cast<uint>(Tp::Contact::PresenceStateYes));
        // He replied the presence request
        QCOMPARE(static_cast<uint>(contact->subscriptionState()),
                    static_cast<uint>(Tp::Contact::PresenceStateYes));
    }

    // Unfortunately there's no other way: let's wait until the account gets created
    QTimer::singleShot(500, mLoop, SLOT(quit()));
    mLoop->exec();

    // Query Nepomuk for all IM accounts that isBuddyOf the accountResource
    QString query = QString("select distinct ?a where { ?a %1 %2 . ?a a %3 . ?r %4 ?a . ?r a %5}")
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::Telepathy::isBuddyOf()))
                            .arg(Soprano::Node::resourceToN3(m_accountResource.uri()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::NCO::IMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::NCO::hasIMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::NCO::PersonContact()));

    Soprano::Model *model = Nepomuk2::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    // Ok now, let's cache all known contacts ids
    QStringList allKnownIds;
    foreach (const Tp::ContactPtr &contact, connection().data()->contactManager()->allKnownContacts()) {
        if (contact.data()->subscriptionState() == Tp::Contact::PresenceStateYes) {
            allKnownIds << contact.data()->id();
        }
    }
    qDebug() << "All known ids: " << allKnownIds;

    for (int i = 0; i < allKnownIds.size(); ++i) {
        // Check that we got some results
        QVERIFY(it.next());
        Nepomuk2::IMAccount foundImAccount(it.binding("a").uri());

        // Check that the IM account only has one ID.
        QStringList accountIDs = foundImAccount.imIDs();

        QCOMPARE(accountIDs.size(), 1);

        // Exactly one ID found. Check if it matches one of the ids provided
        QString accountID = accountIDs.first();

        qDebug() << "Contact found, " << accountID;
        QVERIFY(allKnownIds.contains(accountID));
    }

    // Check that we have no more results
    qDebug() << "End results";
    QVERIFY(!it.next());
}

void ContactTest::testContactRequestAndAuthorize()
{
    // Tp-qt4 test stuff
    QStringList toCheck = QStringList() <<
        QLatin1String("sjoerd@example.com") <<
        QLatin1String("travis@example.com") <<
        QLatin1String("wim@example.com") <<
        QLatin1String("olivier@example.com") <<
        QLatin1String("helen@example.com") <<
        QLatin1String("geraldine@example.com") <<
        QLatin1String("guillaume@example.com") <<
        QLatin1String("christian@example.com") <<
        QLatin1String("test1@kde.org") <<
        QLatin1String("dario.freddi@collabora.co.uk") <<
        QLatin1String("george.grundleborg@collabora.co.uk") <<
        QLatin1String("drf@kde.org");
    QStringList ids;
    QList<Tp::ContactPtr> pendingPublish;
    foreach (const Tp::ContactPtr &contact,
             connection()->contactManager()->allKnownContacts()) {
        qDebug() << " contact:" << contact->id() <<
            "- subscription:" << contact->subscriptionState() <<
            "- publish:" << contact->publishState();
        ids << contact->id();
        if (contact->publishState() == Tp::Contact::PresenceStateAsk) {
            pendingPublish.append(contact);
        }
    }
    ids.sort();
    toCheck.sort();
    QCOMPARE(ids, toCheck);
    QCOMPARE(pendingPublish.size(), 2);

    // Ok now, let's cache all known contacts ids
    QStringList allKnownIds;
    foreach (const Tp::ContactPtr &contact, connection().data()->contactManager()->allKnownContacts()) {
        if (contact.data()->subscriptionState() == Tp::Contact::PresenceStateYes) {
            allKnownIds << contact.data()->id();
        }
    }

    foreach (const Tp::ContactPtr &contact, pendingPublish) {
        QVERIFY(connect(contact.data(),
                        SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
                        mLoop,
                        SLOT(quit())));
        QVERIFY(connect(contact.data(),
                        SIGNAL(publishStateChanged(Tp::Contact::PresenceState)),
                        mLoop,
                        SLOT(quit())));
        contact->requestPresenceSubscription(QLatin1String("please add me"));

        QCOMPARE(mLoop->exec(), 0);
        // I asked to see his presence
        QCOMPARE(static_cast<uint>(contact->subscriptionState()),
                 static_cast<uint>(Tp::Contact::PresenceStateAsk));

        QCOMPARE(mLoop->exec(), 0);

        contact->authorizePresencePublication();

        QCOMPARE(mLoop->exec(), 0);
        QCOMPARE(static_cast<uint>(contact->publishState()),
                 static_cast<uint>(Tp::Contact::PresenceStateYes));
        allKnownIds << contact.data()->id();
    }

    // Unfortunately there's no other way: let's wait until the account gets created
    QTimer::singleShot(500, mLoop, SLOT(quit()));
    mLoop->exec();

    // Query Nepomuk for all IM accounts that isBuddyOf the accountResource
    QString query = QString("select distinct ?a where { ?a %1 %2 . ?a a %3 . ?r %4 ?a . ?r a %5}")
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::Telepathy::isBuddyOf()))
                            .arg(Soprano::Node::resourceToN3(m_accountResource.uri()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::NCO::IMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::NCO::hasIMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::NCO::PersonContact()));

    Soprano::Model *model = Nepomuk2::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    qDebug() << "All known ids: " << allKnownIds;

    for (int i = 0; i < allKnownIds.size(); ++i) {
        // Check that we got some results
        QVERIFY(it.next());
        Nepomuk2::IMAccount foundImAccount(it.binding("a").uri());

        // Check that the IM account only has one ID.
        QStringList accountIDs = foundImAccount.imIDs();

        QCOMPARE(accountIDs.size(), 1);

        // Exactly one ID found. Check if it matches one of the ids provided
        QString accountID = accountIDs.first();

        qDebug() << "Contact found, " << accountID;
        QVERIFY(allKnownIds.contains(accountID));
    }

    // Check that we have no more results
    qDebug() << "End results";
    QVERIFY(!it.next());
}

void ContactTest::testContactRemove()
{
    QStringList ids = QStringList() <<
        QLatin1String("sjoerd@example.com") <<
        QLatin1String("travis@example.com") <<
        QLatin1String("wim@example.com") <<
        QLatin1String("olivier@example.com");

    QList<Tp::ContactPtr> toRemove;
    foreach (const Tp::ContactPtr &contact,
             connection()->contactManager()->allKnownContacts()) {
        qDebug() << " contact:" << contact->id() <<
            "- subscription:" << contact->subscriptionState() <<
            "- publish:" << contact->publishState();
        if (ids.contains(contact->id())) {
            toRemove << contact;
        }
    }
    QCOMPARE(toRemove.size(), 4);

    // Ok now, let's cache all known contacts ids
    QStringList allKnownIds;
    foreach (const Tp::ContactPtr &contact, connection().data()->contactManager()->allKnownContacts()) {
        if (contact.data()->subscriptionState() == Tp::Contact::PresenceStateYes) {
            allKnownIds << contact.data()->id();
        }
    }

    // Remove the presence subscription
    foreach (const Tp::ContactPtr &contact, toRemove) {
        QVERIFY(connect(contact.data(),
                        SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
                        mLoop,
                        SLOT(quit())));
        QVERIFY(connect(contact.data(),
                        SIGNAL(publishStateChanged(Tp::Contact::PresenceState)),
                        mLoop,
                        SLOT(quit())));
        contact->removePresenceSubscription(QLatin1String("go to hell!"));

        QCOMPARE(mLoop->exec(), 0);
        // I asked to see his presence
        QCOMPARE(static_cast<uint>(contact->subscriptionState()),
                 static_cast<uint>(Tp::Contact::PresenceStateNo));

        contact->removePresencePublication();

        QCOMPARE(mLoop->exec(), 0);
        QCOMPARE(static_cast<uint>(contact->publishState()),
                 static_cast<uint>(Tp::Contact::PresenceStateNo));
        allKnownIds.removeOne(contact.data()->id());
    }

    // Unfortunately there's no other way: let's wait until the account gets created
    QTimer::singleShot(500, mLoop, SLOT(quit()));
    mLoop->exec();

    // Query Nepomuk for all IM accounts that isBuddyOf the accountResource
    QString query = QString("select distinct ?a where { ?a %1 %2 . ?a a %3 . ?r %4 ?a . ?r a %5}")
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::Telepathy::isBuddyOf()))
                            .arg(Soprano::Node::resourceToN3(m_accountResource.uri()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::NCO::IMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::NCO::hasIMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk2::Vocabulary::NCO::PersonContact()));

    Soprano::Model *model = Nepomuk2::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    qDebug() << "All known ids: " << allKnownIds;

    for (int i = 0; i < allKnownIds.size(); ++i) {
        // Check that we got some results
        QVERIFY(it.next());
        Nepomuk2::IMAccount foundImAccount(it.binding("a").uri());

        // Check that the IM account only has one ID.
        QStringList accountIDs = foundImAccount.imIDs();

        QCOMPARE(accountIDs.size(), 1);

        // Exactly one ID found. Check if it matches one of the ids provided
        QString accountID = accountIDs.first();

        qDebug() << "Contact found, " << accountID;
        QVERIFY(allKnownIds.contains(accountID));
    }

    // Check that we have no more results
    qDebug() << "End results";
    QVERIFY(!it.next());
}

void ContactTest::cleanupTestCase()
{
    cleanupTestCaseConnectionImpl();
}

QTEST_MAIN(ContactTest)
#include "contact-test.moc"
