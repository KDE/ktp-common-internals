/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2011 Collabora Ltd. <info@collabora.co.uk>
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

#include "storage-test.h"

#include "test-backdoors.h"

#include "ontologies/contactgroup.h"
#include "ontologies/pimo.h"

#include <KDebug>

#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Thing>

#include <qtest_kde.h>

StorageTest::StorageTest(QObject *parent)
: Test(parent),
  m_storage(0)
{
    kDebug();
}

StorageTest::~StorageTest()
{
    kDebug();
}

void StorageTest::initTestCase()
{
    initTestCaseImpl();
}

void StorageTest::init()
{
    initImpl();
}

void StorageTest::testConstructorDestructor()
{
    // First test constructing the NepomukStorage on a Nepomuk database with no relevant
    // data already in it.
    m_storage = new NepomukStorage(this);

    // Check that the Nepomuk mePersonContact has been created.
    QVERIFY(TestBackdoors::nepomukStorageMePersonContact(m_storage).exists());

    // Check that the PersonContact and IMAccount lists are empty
    QVERIFY(TestBackdoors::nepomukStorageAccounts(m_storage)->isEmpty());
    QVERIFY(TestBackdoors::nepomukStorageContacts(m_storage)->isEmpty());

    // Now destroy the NepomukStorage, running the event loop
    // to make sure the destruction is completed.
    connect(m_storage, SIGNAL(destroyed(QObject*)), mLoop, SLOT(quit()));
    m_storage->deleteLater();
    m_storage = 0;
    QCOMPARE(mLoop->exec(), 0);

    // Next, try constructing a NepomukStorage instance where the mePersonContact already exists.
    m_storage = new NepomukStorage(this);

    // Check that the Nepomuk mePersonContact has been created.
    QVERIFY(TestBackdoors::nepomukStorageMePersonContact(m_storage).exists());

    // Check that the PersonContact and IMAccount lists are empty
    QVERIFY(TestBackdoors::nepomukStorageAccounts(m_storage)->isEmpty());
    QVERIFY(TestBackdoors::nepomukStorageContacts(m_storage)->isEmpty());

    // Now destroy the NepomukStorage, running the event loop
    // to make sure the destruction is completed.
    connect(m_storage, SIGNAL(destroyed(QObject*)), mLoop, SLOT(quit()));
    m_storage->deleteLater();
    m_storage = 0;
    QCOMPARE(mLoop->exec(), 0);
}

void StorageTest::testCreateAccount()
{
    m_storage = new NepomukStorage(this);
    Nepomuk::PersonContact mePersonContact = TestBackdoors::nepomukStorageMePersonContact(m_storage);
    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);

    QVERIFY(m_storage);

    QVERIFY(mePersonContact.exists());
    QCOMPARE(mePersonContact.iMAccounts().size(), 0);

    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 0);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // Test creating an account which is not already in Nepomuk.
    // Issue the command to the storage
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // Check its properties are correct
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());
    QCOMPARE(imAcc1.imIDs().size(), 1);
    QCOMPARE(imAcc1.imIDs().first(), QLatin1String("foo@bar.baz"));
    QCOMPARE(imAcc1.accountIdentifiers().size(), 1);
    QCOMPARE(imAcc1.accountIdentifiers().first(), QLatin1String("/foo/bar/baz"));
    QCOMPARE(imAcc1.imAccountTypes().size(), 1);
    QCOMPARE(imAcc1.imAccountTypes().first(), QLatin1String("test"));

    // Check that it is correctly related to the mePersonContact.
    QCOMPARE(mePersonContact.iMAccounts().size(), 1);

    // Test creating an account which *is* already in Nepomuk.
    // Add the account to Nepomuk.
    Nepomuk::IMAccount imAcc2;
    imAcc2.setAccountIdentifiers(QStringList() << QLatin1String("/foo/bar/baz/bong"));
    imAcc2.setImIDs(QStringList() << QLatin1String("foo.bar@baz.bong"));
    imAcc2.setImAccountTypes(QStringList() << QLatin1String("test"));
    QVERIFY(imAcc2.exists());
    mePersonContact.addIMAccount(imAcc2);

    // Now tell the storage about that account.
    m_storage->createAccount(QLatin1String("/foo/bar/baz/bong"),
                             QLatin1String("foo.bar@baz.bong"),
                             QLatin1String("test"));

    // Check the account is found.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 2);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // Check its properties are correct
    Nepomuk::IMAccount imAcc3 = accounts->value(QLatin1String("/foo/bar/baz/bong"));
    QVERIFY(imAcc3.exists());
    QCOMPARE(imAcc3.imIDs().size(), 1);
    QCOMPARE(imAcc3.imIDs().first(), QLatin1String("foo.bar@baz.bong"));
    QCOMPARE(imAcc3.accountIdentifiers().size(), 1);
    QCOMPARE(imAcc3.accountIdentifiers().first(), QLatin1String("/foo/bar/baz/bong"));
    QCOMPARE(imAcc3.imAccountTypes().size(), 1);
    QCOMPARE(imAcc3.imAccountTypes().first(), QLatin1String("test"));
    QCOMPARE(imAcc2, imAcc3);

    // Check that it is correctly related to the mePersonContact.
    QCOMPARE(mePersonContact.iMAccounts().size(), 2);

    // Test creating an account twice (by recreating the first account we created).
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 2);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // Check its properties are correct
    Nepomuk::IMAccount imAcc4 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc4.exists());
    QCOMPARE(imAcc4.imIDs().size(), 1);
    QCOMPARE(imAcc4.imIDs().first(), QLatin1String("foo@bar.baz"));
    QCOMPARE(imAcc4.accountIdentifiers().size(), 1);
    QCOMPARE(imAcc4.accountIdentifiers().first(), QLatin1String("/foo/bar/baz"));
    QCOMPARE(imAcc4.imAccountTypes().size(), 1);
    QCOMPARE(imAcc4.imAccountTypes().first(), QLatin1String("test"));
    QCOMPARE(imAcc4, imAcc1);

    // Check that it is correctly related to the mePersonContact.
    QCOMPARE(mePersonContact.iMAccounts().size(), 2);

    // Question: Should we test for cases where there is data in Nepomuk that would
    // partially match the query but shouldn't actually match because it is not a complete
    // set of data wrt telepathy?
    // Answer: for now, I don't think there's any point - but if an error in the query is found in
    // the future, then of course we should add a test for that error here to avoid it
    // regressing in the future (grundleborg).

    // Cleanup the nepomuk resources created in this test case.
    imAcc1.remove();
    imAcc2.remove();
    imAcc3.remove();
    imAcc4.remove();
}

void StorageTest::testDestroyAccount()
{
    // Create the Storage.
    m_storage = new NepomukStorage(this);
    QVERIFY(m_storage);

    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);

    // Create an account on the storage.
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And in Nepomuk...
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());

    // Now destroy the account.
    m_storage->destroyAccount(QLatin1String("/foo/bar/baz"));

    // The account remains in the list despite being destroyed.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And it should still exist in Nepomuk.
    QVERIFY(imAcc1.exists());

    // However, its presence changes.
    QCOMPARE(imAcc1.imStatus(), QLatin1String("unknown"));
    QCOMPARE(imAcc1.statusTypes().size(), 1);
    QCOMPARE(imAcc1.statusTypes().first(), (long long)Tp::ConnectionPresenceTypeUnknown);

    // Cleanup the nepomuk resources created in this test case.
    imAcc1.remove();
}

void StorageTest::testSetAccountNickname()
{
    // Create the Storage.
    m_storage = new NepomukStorage(this);
    QVERIFY(m_storage);

    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);

    // Create an account on the storage.
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And in Nepomuk...
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());

    // Check the nickname before we set it for the first time.
    QCOMPARE(imAcc1.imNicknames().size(), 0);

    // Set the nickname.
    m_storage->setAccountNickname(QLatin1String("/foo/bar/baz"),
                                  QLatin1String("Test Nickname"));

    // Check Nepomuk resource contains the appropriate data.
    QCOMPARE(imAcc1.imNicknames().size(), 1);
    QCOMPARE(imAcc1.imNicknames().first(), QLatin1String("Test Nickname"));

    // Change the Nickname.
    m_storage->setAccountNickname(QLatin1String("/foo/bar/baz"),
                                  QLatin1String("Test Nickname Changed"));

    // Check the Nepomuk resource contains the appropriate data.
    QCOMPARE(imAcc1.imNicknames().size(), 1);
    QCOMPARE(imAcc1.imNicknames().first(), QLatin1String("Test Nickname Changed"));

    // Cleanup the nepomuk resources created in this test case.
    imAcc1.remove();
}

void StorageTest::testSetAccountCurrentPresence()
{
    // Create the Storage.
    m_storage = new NepomukStorage(this);
    QVERIFY(m_storage);

    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);

    // Create an account on the storage.
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And in Nepomuk...
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());

    // Check the presence properties before we set them.
    QVERIFY(imAcc1.imStatus().isEmpty());
    QCOMPARE(imAcc1.imStatusMessages().size(), 0);
    QCOMPARE(imAcc1.statusTypes().size(), 0);

    // Set the presence.
    Tp::SimplePresence p1;
    p1.status = "away";
    p1.statusMessage = "Hello";
    p1.type = 4;
    m_storage->setAccountCurrentPresence(QLatin1String("/foo/bar/baz"), p1);

    // Check the nepomuk resources are correct.
    QCOMPARE(imAcc1.imStatus(), QLatin1String("away"));
    QCOMPARE(imAcc1.imStatusMessages().size(), 1);
    QCOMPARE(imAcc1.imStatusMessages().first(), QLatin1String("Hello"));
    QCOMPARE(imAcc1.statusTypes().size(), 1);
    QCOMPARE(imAcc1.statusTypes().first(), (long long)4);

    // Set the presence.
    Tp::SimplePresence p2;
    p2.status = "available";
    p2.statusMessage = "Bye";
    p2.type = 1;
    m_storage->setAccountCurrentPresence(QLatin1String("/foo/bar/baz"), p2);

    // Check the nepomuk resources are correct.
    QCOMPARE(imAcc1.imStatus(), QLatin1String("available"));
    QCOMPARE(imAcc1.imStatusMessages().size(), 1);
    QCOMPARE(imAcc1.imStatusMessages().first(), QLatin1String("Bye"));
    QCOMPARE(imAcc1.statusTypes().size(), 1);
    QCOMPARE(imAcc1.statusTypes().first(), (long long)1);

    // Cleanup the Nepomuk Resources used in this test
    imAcc1.remove();
}

void StorageTest::testCreateContact()
{
    // Create the Storage.
    m_storage = new NepomukStorage(this);
    QVERIFY(m_storage);

    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);
    QHash<ContactIdentifier, ContactResources> *contacts = TestBackdoors::nepomukStorageContacts(m_storage);

    // Create an account on the storage.
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And in Nepomuk...
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());
    QCOMPARE(imAcc1.isAccessedByOf().size(), 0);

    // Check there is only 1 PIMO Person in Nepomuk.
    QCOMPARE(Nepomuk::ResourceManager::instance()->allResourcesOfType(Nepomuk::Vocabulary::PIMO::Person()).size(), 1);

    // Test 1: Create a contact which doesn't already exist.
    m_storage->createContact(QLatin1String("/foo/bar/baz"),
                             QLatin1String("test@remote-contact.com"));

    // Check the Contact is created.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 1);

    // Check its identifier is correct.
    ContactIdentifier cId2(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"));
    QVERIFY(contacts->contains(cId2));

    // Check the Nepomuk resources are created correctly.
    ContactResources cRes2 = contacts->value(cId2);
    Nepomuk::IMAccount imAcc2 = cRes2.imAccount();
    Nepomuk::PersonContact pC2 = cRes2.personContact();
    QVERIFY(imAcc2.exists());
    QVERIFY(pC2.exists());
    QCOMPARE(imAcc2.imStatus(), QLatin1String("unknown"));
    QCOMPARE(imAcc2.imIDs().size(), 1);
    QCOMPARE(imAcc2.imIDs().first(), QLatin1String("test@remote-contact.com"));
    QCOMPARE(imAcc2.statusTypes().size(), 1);
    QCOMPARE(imAcc2.statusTypes().first(), (long long)Tp::ConnectionPresenceTypeUnknown);
    QCOMPARE(imAcc2.imAccountTypes().size(), 1);
    QCOMPARE(imAcc2.imAccountTypes().first(), QLatin1String("test"));
    QCOMPARE(imAcc1.isAccessedByOf().size(), 1);
    QCOMPARE(imAcc2.isAccessedBys().size(), 1);
    QCOMPARE(imAcc2.isAccessedBys().first(), imAcc1);
    QCOMPARE(pC2.iMAccounts().size(), 1);
    QCOMPARE(pC2.iMAccounts().first(), imAcc2);

    // Check the PIMO Person.
    QCOMPARE(Nepomuk::ResourceManager::instance()->allResourcesOfType(Nepomuk::Vocabulary::PIMO::Person()).size(), 2);

    foreach (Nepomuk::Resource r, Nepomuk::ResourceManager::instance()->allResourcesOfType(Nepomuk::Vocabulary::PIMO::Person())) {
        if (r != Nepomuk::Resource("nepomuk:/myself")) {
            Nepomuk::Thing t(r);
            QVERIFY(t.groundingOccurrences().contains(pC2));
        }
    }

    // Test 2: Create a contact which already exists in Nepomuk.
    // Pre-populate Nepomuk with a valid contact
    Nepomuk::IMAccount imAcc3;
    Nepomuk::PersonContact pC3;
    imAcc3.setImStatus("away");
    imAcc3.setImIDs(QStringList() << "test2@remote-contact.com");
    imAcc3.setStatusTypes(QList<long long int>() << Tp::ConnectionPresenceTypeAway);
    imAcc3.setImAccountTypes(QStringList() << "test");
    imAcc3.addIsAccessedBy(imAcc1);
    pC3.addIMAccount(imAcc3);

    // Check the pre-population worked.
    QVERIFY(imAcc3.exists());
    QVERIFY(pC3.exists());
    QCOMPARE(imAcc3.imStatus(), QLatin1String("away"));
    QCOMPARE(imAcc3.imIDs().size(), 1);
    QCOMPARE(imAcc3.imIDs().first(), QLatin1String("test2@remote-contact.com"));
    QCOMPARE(imAcc3.statusTypes().size(), 1);
    QCOMPARE(imAcc3.statusTypes().first(), (long long)Tp::ConnectionPresenceTypeAway);
    QCOMPARE(imAcc3.imAccountTypes().size(), 1);
    QCOMPARE(imAcc3.imAccountTypes().first(), QLatin1String("test"));
    QCOMPARE(imAcc1.isAccessedByOf().size(), 2);
    QCOMPARE(imAcc3.isAccessedBys().size(), 1);
    QCOMPARE(imAcc3.isAccessedBys().first(), imAcc1);
    QCOMPARE(pC3.iMAccounts().size(), 1);
    QCOMPARE(pC3.iMAccounts().first(), imAcc3);

    // Tell the storage about the contact
    m_storage->createContact(QLatin1String("/foo/bar/baz"),
                             QLatin1String("test2@remote-contact.com"));

    // Check that the contact was added to the storage.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 2);

    // Check its identifier is correct.
    ContactIdentifier cId4(QLatin1String("/foo/bar/baz"), QLatin1String("test2@remote-contact.com"));
    QVERIFY(contacts->contains(cId4));

    // Check the Nepomuk resources still have the right values.
    ContactResources cRes4 = contacts->value(cId4);
    Nepomuk::IMAccount imAcc4 = cRes4.imAccount();
    Nepomuk::PersonContact pC4 = cRes4.personContact();
    QCOMPARE(imAcc4, imAcc3);
    QCOMPARE(pC4, pC3);
    QVERIFY(imAcc3.exists());
    QVERIFY(pC3.exists());
    QCOMPARE(imAcc3.imStatus(), QLatin1String("away"));
    QCOMPARE(imAcc3.imIDs().size(), 1);
    QCOMPARE(imAcc3.imIDs().first(), QLatin1String("test2@remote-contact.com"));
    QCOMPARE(imAcc3.statusTypes().size(), 1);
    QCOMPARE(imAcc3.statusTypes().first(), (long long)Tp::ConnectionPresenceTypeAway);
    QCOMPARE(imAcc3.imAccountTypes().size(), 1);
    QCOMPARE(imAcc3.imAccountTypes().first(), QLatin1String("test"));
    QCOMPARE(imAcc1.isAccessedByOf().size(), 2);
    QCOMPARE(imAcc3.isAccessedBys().size(), 1);
    QCOMPARE(imAcc3.isAccessedBys().first(), imAcc1);
    QCOMPARE(pC3.iMAccounts().size(), 1);
    QCOMPARE(pC3.iMAccounts().first(), imAcc3);

    // Test 3: Create a contact twice.
    m_storage->createContact(QLatin1String("/foo/bar/baz"),
                             QLatin1String("test@remote-contact.com"));

    // Check the Contact is created.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 2);

    // Check its identifier is correct.
    ContactIdentifier cId5(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"));
    QVERIFY(contacts->contains(cId5));

    // Check the Nepomuk resources are created correctly.
    ContactResources cRes5 = contacts->value(cId5);
    Nepomuk::IMAccount imAcc5 = cRes5.imAccount();
    Nepomuk::PersonContact pC5 = cRes5.personContact();
    QCOMPARE(imAcc5, imAcc2);
    QCOMPARE(pC5, pC2);
    QVERIFY(imAcc2.exists());
    QVERIFY(pC2.exists());
    QCOMPARE(imAcc2.imStatus(), QLatin1String("unknown"));
    QCOMPARE(imAcc2.imIDs().size(), 1);
    QCOMPARE(imAcc2.imIDs().first(), QLatin1String("test@remote-contact.com"));
    QCOMPARE(imAcc2.statusTypes().size(), 1);
    QCOMPARE(imAcc2.statusTypes().first(), (long long)Tp::ConnectionPresenceTypeUnknown);
    QCOMPARE(imAcc2.imAccountTypes().size(), 1);
    QCOMPARE(imAcc2.imAccountTypes().first(), QLatin1String("test"));
    QCOMPARE(imAcc1.isAccessedByOf().size(), 2);
    QCOMPARE(imAcc2.isAccessedBys().size(), 1);
    QCOMPARE(imAcc2.isAccessedBys().first(), imAcc1);
    QCOMPARE(pC2.iMAccounts().size(), 1);
    QCOMPARE(pC2.iMAccounts().first(), imAcc2);

    // Cleanup Nepomuk Resources used in this test case.
    imAcc1.remove();
    imAcc2.remove();
    pC2.remove();
    imAcc3.remove();
    pC3.remove();
    imAcc4.remove();
    pC4.remove();
    imAcc5.remove();
    pC5.remove();
}

void StorageTest::testDestroyContact()
{
    // Create the Storage.
    m_storage = new NepomukStorage(this);
    QVERIFY(m_storage);

    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);
    QHash<ContactIdentifier, ContactResources> *contacts = TestBackdoors::nepomukStorageContacts(m_storage);

    // Create an account on the storage.
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And in Nepomuk...
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());
    QCOMPARE(imAcc1.isAccessedByOf().size(), 0);

    // Create a contact
    m_storage->createContact(QLatin1String("/foo/bar/baz"),
                             QLatin1String("test@remote-contact.com"));

    // Check the Contact is created.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 1);

    // Check its identifier is correct.
    ContactIdentifier cId2(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"));
    QVERIFY(contacts->contains(cId2));

    // Check the Nepomuk resources are created correctly.
    ContactResources cRes2 = contacts->value(cId2);
    Nepomuk::IMAccount imAcc2 = cRes2.imAccount();
    Nepomuk::PersonContact pC2 = cRes2.personContact();
    QVERIFY(imAcc2.exists());
    QVERIFY(pC2.exists());

    // Now destroy the contact
    m_storage->destroyContact(QLatin1String("/foo/bar/baz"),
                              QLatin1String("test@remote-contact.com"));

    // The contact should still be in the list
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 1);
    QVERIFY(contacts->contains(cId2));

    // And still in Nepomuk
    QVERIFY(imAcc2.exists());
    QVERIFY(pC2.exists());

    // It's presence should be unknown now
    QCOMPARE(imAcc2.imStatus(), QLatin1String("unknown"));
    QCOMPARE(imAcc2.statusTypes().size(), 1);
    QCOMPARE(imAcc2.statusTypes().first(), (long long)Tp::ConnectionPresenceTypeUnknown);

    // Cleanup Nepomuk Resources used in this test
    imAcc1.remove();
    imAcc2.remove();
    pC2.remove();
}

void StorageTest::testSetContactAlias()
{
    // Create the Storage.
    m_storage = new NepomukStorage(this);
    QVERIFY(m_storage);

    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);
    QHash<ContactIdentifier, ContactResources> *contacts = TestBackdoors::nepomukStorageContacts(m_storage);

    // Create an account on the storage.
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And in Nepomuk...
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());
    QCOMPARE(imAcc1.isAccessedByOf().size(), 0);

    // Create a contact
    m_storage->createContact(QLatin1String("/foo/bar/baz"),
                             QLatin1String("test@remote-contact.com"));

    // Check the Contact is created.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 1);

    // Check its identifier is correct.
    ContactIdentifier cId2(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"));
    QVERIFY(contacts->contains(cId2));

    // Check the Nepomuk resources are created correctly.
    ContactResources cRes2 = contacts->value(cId2);
    Nepomuk::IMAccount imAcc2 = cRes2.imAccount();
    Nepomuk::PersonContact pC2 = cRes2.personContact();
    QVERIFY(imAcc2.exists());
    QVERIFY(pC2.exists());

    // Check the Alias is empty.
    QCOMPARE(imAcc2.imNicknames().size(), 0);

    // Set the alias of the contact.
    m_storage->setContactAlias(QLatin1String("/foo/bar/baz"),
                               QLatin1String("test@remote-contact.com"),
                               QLatin1String("Test Alias 1"));

    // Check the alias now.
    QCOMPARE(imAcc2.imNicknames().size(), 1);
    QCOMPARE(imAcc2.imNicknames().first(), QLatin1String("Test Alias 1"));

    // Change the alias of the contact.
    m_storage->setContactAlias(QLatin1String("/foo/bar/baz"),
                               QLatin1String("test@remote-contact.com"),
                               QLatin1String("Test Alias 2"));

    // Check the alias now.
    QCOMPARE(imAcc2.imNicknames().size(), 1);
    QCOMPARE(imAcc2.imNicknames().first(), QLatin1String("Test Alias 2"));

    // Cleanup the Nepomuk resources used in this test.
    imAcc1.remove();
    imAcc2.remove();
    pC2.remove();
}

void StorageTest::testSetContactPresence()
{
    // Create the Storage.
    m_storage = new NepomukStorage(this);
    QVERIFY(m_storage);

    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);
    QHash<ContactIdentifier, ContactResources> *contacts = TestBackdoors::nepomukStorageContacts(m_storage);

    // Create an account on the storage.
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And in Nepomuk...
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());
    QCOMPARE(imAcc1.isAccessedByOf().size(), 0);

    // Create a contact
    m_storage->createContact(QLatin1String("/foo/bar/baz"),
                             QLatin1String("test@remote-contact.com"));

    // Check the Contact is created.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 1);

    // Check its identifier is correct.
    ContactIdentifier cId2(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"));
    QVERIFY(contacts->contains(cId2));

    // Check the Nepomuk resources are created correctly.
    ContactResources cRes2 = contacts->value(cId2);
    Nepomuk::IMAccount imAcc2 = cRes2.imAccount();
    Nepomuk::PersonContact pC2 = cRes2.personContact();
    QVERIFY(imAcc2.exists());
    QVERIFY(pC2.exists());

    // Check the Presence is default.
    QCOMPARE(imAcc2.imStatus(), QLatin1String("unknown"));
    QCOMPARE(imAcc2.imStatusMessages().size(), 0);
    QCOMPARE(imAcc2.statusTypes().size(), 1);
    QCOMPARE(imAcc2.statusTypes().first(), (long long)Tp::ConnectionPresenceTypeUnknown);

    // Set the presence of the contact.
    Tp::SimplePresence p1;
    p1.status = QLatin1String("available");
    p1.statusMessage = QLatin1String("foo");
    p1.type = Tp::ConnectionPresenceTypeAvailable;
    m_storage->setContactPresence(QLatin1String("/foo/bar/baz"),
                                  QLatin1String("test@remote-contact.com"),
                                  p1);

    // Check the presence now.
    QCOMPARE(imAcc2.imStatus(), QLatin1String("available"));
    QCOMPARE(imAcc2.imStatusMessages().size(), 1);
    QCOMPARE(imAcc2.imStatusMessages().first(), QLatin1String("foo"));
    QCOMPARE(imAcc2.statusTypes().size(), 1);
    QCOMPARE(imAcc2.statusTypes().first(), (long long)Tp::ConnectionPresenceTypeAvailable);

    // Change the presence of the contact.
    Tp::SimplePresence p2;
    p2.status = QLatin1String("away");
    p2.statusMessage = QLatin1String("bar");
    p2.type = Tp::ConnectionPresenceTypeAway;
    m_storage->setContactPresence(QLatin1String("/foo/bar/baz"),
                                  QLatin1String("test@remote-contact.com"),
                                  p2);

    // Check the presence now.
    QCOMPARE(imAcc2.imStatus(), QLatin1String("away"));
    QCOMPARE(imAcc2.imStatusMessages().size(), 1);
    QCOMPARE(imAcc2.imStatusMessages().first(), QLatin1String("bar"));
    QCOMPARE(imAcc2.statusTypes().size(), 1);
    QCOMPARE(imAcc2.statusTypes().first(), (long long)Tp::ConnectionPresenceTypeAway);

    // Change the presence to one without a message
    Tp::SimplePresence p3;
    p3.status = QLatin1String("offline");
    p3.type = Tp::ConnectionPresenceTypeOffline;
    m_storage->setContactPresence(QLatin1String("/foo/bar/baz"),
                                  QLatin1String("test@remote-contact.com"),
                                  p3);

    // Check the presence now.
    QCOMPARE(imAcc2.imStatus(), QLatin1String("offline"));
    QCOMPARE(imAcc2.imStatusMessages().size(), 0);
    QCOMPARE(imAcc2.statusTypes().size(), 1);
    QCOMPARE(imAcc2.statusTypes().first(), (long long)Tp::ConnectionPresenceTypeOffline);

    // Cleanup the Nepomuk resources used in this test.
    imAcc1.remove();
    imAcc2.remove();
    pC2.remove();
}

void StorageTest::testSetContactGroups()
{
    // Create the Storage.
    m_storage = new NepomukStorage(this);
    QVERIFY(m_storage);

    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);
    QHash<ContactIdentifier, ContactResources> *contacts = TestBackdoors::nepomukStorageContacts(m_storage);

    // Create an account on the storage.
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And in Nepomuk...
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());
    QCOMPARE(imAcc1.isAccessedByOf().size(), 0);

    // Create a contact
    m_storage->createContact(QLatin1String("/foo/bar/baz"),
                             QLatin1String("test@remote-contact.com"));

    // Check the Contact is created.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 1);

    // Check its identifier is correct.
    ContactIdentifier cId2(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"));
    QVERIFY(contacts->contains(cId2));

    // Check the Nepomuk resources are created correctly.
    ContactResources cRes2 = contacts->value(cId2);
    Nepomuk::IMAccount imAcc2 = cRes2.imAccount();
    Nepomuk::PersonContact pC2 = cRes2.personContact();
    QVERIFY(imAcc2.exists());
    QVERIFY(pC2.exists());

    // Check that the groups property is initially empty.
    QCOMPARE(pC2.belongsToGroups().size(), 0);
    QCOMPARE(Nepomuk::ContactGroup::allContactGroups().size(), 0);

    // Add to a group that doesn't already exist
    m_storage->setContactGroups(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"),
                                QStringList() << QLatin1String("testgroup1"));

    // Add to a group we're already in
    QCOMPARE(Nepomuk::ContactGroup::allContactGroups().size(), 1);
    QCOMPARE(pC2.belongsToGroups().size(), 1);
    QCOMPARE(pC2.belongsToGroups().first().contactGroupName(), QLatin1String("testgroup1"));

    // Create a group in Nepomuk so we can try adding ourselves to an already existing group and
    // check it succeeds.
    Nepomuk::ContactGroup g2;
    QVERIFY(!g2.exists());
    g2.setContactGroupName(QLatin1String("testgroup2"));
    QCOMPARE(g2.contactGroupName(), QLatin1String("testgroup2"));
    QVERIFY(g2.exists());
    QCOMPARE(Nepomuk::ContactGroup::allContactGroups().size(), 2);

    // Add the contact to that group.
    m_storage->setContactGroups(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"),
                                QStringList() << QLatin1String("testgroup1") << QLatin1String("testgroup2"));

    // Check it worked OK
    QCOMPARE(Nepomuk::ContactGroup::allContactGroups().size(), 2);
    QCOMPARE(pC2.belongsToGroups().size(), 2);
    QVERIFY(!(pC2.belongsToGroups().at(0) == g2) ^ !(pC2.belongsToGroups().at(1) == g2));

    // Remove from a group
    m_storage->setContactGroups(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"),
                                QStringList() << QLatin1String("testgroup1"));

    // Check it worked OK
    QCOMPARE(Nepomuk::ContactGroup::allContactGroups().size(), 2);
    QCOMPARE(pC2.belongsToGroups().size(), 1);
    QCOMPARE(pC2.belongsToGroups().first().contactGroupName(), QLatin1String("testgroup1"));

    // Add ourselves to two groups, one of which doesn't yet exist
    m_storage->setContactGroups(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"),
                                QStringList() << QLatin1String("testgroup1") << QLatin1String("testgroup2")
                                              << QLatin1String("testgroup3"));

    // Check it worked OK
    QCOMPARE(Nepomuk::ContactGroup::allContactGroups().size(), 3);
    QCOMPARE(pC2.belongsToGroups().size(), 3);

    // Remove ourselves from two groups
    m_storage->setContactGroups(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"),
                                QStringList() << QLatin1String("testgroup2")
                                              << QLatin1String("testgroup3"));

    // Check it worked OK
    QCOMPARE(Nepomuk::ContactGroup::allContactGroups().size(), 3);
    QCOMPARE(pC2.belongsToGroups().size(), 2);

    // Cleanup
    imAcc1.remove();
    imAcc2.remove();
    pC2.remove();

    foreach (Nepomuk::ContactGroup g, Nepomuk::ContactGroup::allContactGroups()) {
        g.remove();
    }
}

void StorageTest::testSetContactBlockedStatus()
{
    // Create the Storage.
    m_storage = new NepomukStorage(this);
    QVERIFY(m_storage);

    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);
    QHash<ContactIdentifier, ContactResources> *contacts = TestBackdoors::nepomukStorageContacts(m_storage);

    // Create an account on the storage.
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And in Nepomuk...
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());
    QCOMPARE(imAcc1.isAccessedByOf().size(), 0);

    // Create a contact
    m_storage->createContact(QLatin1String("/foo/bar/baz"),
                             QLatin1String("test@remote-contact.com"));

    // Check the Contact is created.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 1);

    // Check its identifier is correct.
    ContactIdentifier cId2(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"));
    QVERIFY(contacts->contains(cId2));

    // Check the Nepomuk resources are created correctly.
    ContactResources cRes2 = contacts->value(cId2);
    Nepomuk::IMAccount imAcc2 = cRes2.imAccount();
    Nepomuk::PersonContact pC2 = cRes2.personContact();
    QVERIFY(imAcc2.exists());
    QVERIFY(pC2.exists());

    // Check the default blocked status.
    QCOMPARE(imAcc2.isBlockeds().size(), 0);

    // Block the account.
    m_storage->setContactBlockStatus(QLatin1String("/foo/bar/baz"),
                                     QLatin1String("test@remote-contact.com"),
                                     true);

    // Check the Nepomuk resources
    QCOMPARE(imAcc2.isBlockeds().size(), 1);
    QCOMPARE(imAcc2.isBlockeds().first(), true);

    // Unblock it
    m_storage->setContactBlockStatus(QLatin1String("/foo/bar/baz"),
                                     QLatin1String("test@remote-contact.com"),
                                     false);

    // Check the Nepomuk resources
    QCOMPARE(imAcc2.isBlockeds().size(), 1);
    QCOMPARE(imAcc2.isBlockeds().first(), false);

    // Cleanup Nepomuk resources used in this test.
    imAcc1.remove();
    imAcc2.remove();
    pC2.remove();
}

void StorageTest::testSetContactPublishState()
{
    // Create the Storage.
    m_storage = new NepomukStorage(this);
    QVERIFY(m_storage);

    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);
    QHash<ContactIdentifier, ContactResources> *contacts = TestBackdoors::nepomukStorageContacts(m_storage);

    // Create an account on the storage.
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And in Nepomuk...
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());
    QCOMPARE(imAcc1.isAccessedByOf().size(), 0);

    // Create a contact
    m_storage->createContact(QLatin1String("/foo/bar/baz"),
                             QLatin1String("test@remote-contact.com"));

    // Check the Contact is created.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 1);

    // Check its identifier is correct.
    ContactIdentifier cId2(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"));
    QVERIFY(contacts->contains(cId2));

    // Check the Nepomuk resources are created correctly.
    ContactResources cRes2 = contacts->value(cId2);
    Nepomuk::IMAccount imAcc2 = cRes2.imAccount();
    Nepomuk::PersonContact pC2 = cRes2.personContact();
    QVERIFY(imAcc2.exists());
    QVERIFY(pC2.exists());

    // Check the default publish state
    QCOMPARE(imAcc2.publishesPresenceTos().size(), 0);
    QCOMPARE(imAcc1.requestedPresenceSubscriptionTos().size(), 0);

    // Set the publish state to Yes
    m_storage->setContactPublishState(QLatin1String("/foo/bar/baz"),
                                      QLatin1String("test@remote-contact.com"),
                                      Tp::Contact::PresenceStateYes);

    // Check
    QCOMPARE(imAcc2.publishesPresenceTos().size(), 1);
    QCOMPARE(imAcc2.publishesPresenceTos().first(), imAcc1);
    QCOMPARE(imAcc1.requestedPresenceSubscriptionTos().size(), 0);

    // Set the publish state to Request
    m_storage->setContactPublishState(QLatin1String("/foo/bar/baz"),
                                      QLatin1String("test@remote-contact.com"),
                                      Tp::Contact::PresenceStateAsk);

    // Check
    QCOMPARE(imAcc2.publishesPresenceTos().size(), 0);
    QCOMPARE(imAcc1.requestedPresenceSubscriptionTos().size(), 1);
    QCOMPARE(imAcc1.requestedPresenceSubscriptionTos().first(), imAcc2);

    // Set the publish state to no
    m_storage->setContactPublishState(QLatin1String("/foo/bar/baz"),
                                      QLatin1String("test@remote-contact.com"),
                                      Tp::Contact::PresenceStateNo);

    // Check
    QCOMPARE(imAcc2.publishesPresenceTos().size(), 0);
    QCOMPARE(imAcc1.requestedPresenceSubscriptionTos().size(), 0);

    // Cleanup Nepomuk resources used in this test.
    imAcc1.remove();
    imAcc2.remove();
    pC2.remove();
}

void StorageTest::testSetContactSubscriptionState()
{
    // Create the Storage.
    m_storage = new NepomukStorage(this);
    QVERIFY(m_storage);

    QHash<QString, Nepomuk::IMAccount> *accounts = TestBackdoors::nepomukStorageAccounts(m_storage);
    QHash<ContactIdentifier, ContactResources> *contacts = TestBackdoors::nepomukStorageContacts(m_storage);

    // Create an account on the storage.
    m_storage->createAccount(QLatin1String("/foo/bar/baz"),
                             QLatin1String("foo@bar.baz"),
                             QLatin1String("test"));

    // Check the Account is created
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 0);

    // And in Nepomuk...
    Nepomuk::IMAccount imAcc1 = accounts->value(QLatin1String("/foo/bar/baz"));
    QVERIFY(imAcc1.exists());
    QCOMPARE(imAcc1.isAccessedByOf().size(), 0);

    // Create a contact
    m_storage->createContact(QLatin1String("/foo/bar/baz"),
                             QLatin1String("test@remote-contact.com"));

    // Check the Contact is created.
    QCOMPARE(TestBackdoors::nepomukStorageAccounts(m_storage)->size(), 1);
    QCOMPARE(TestBackdoors::nepomukStorageContacts(m_storage)->size(), 1);

    // Check its identifier is correct.
    ContactIdentifier cId2(QLatin1String("/foo/bar/baz"), QLatin1String("test@remote-contact.com"));
    QVERIFY(contacts->contains(cId2));

    // Check the Nepomuk resources are created correctly.
    ContactResources cRes2 = contacts->value(cId2);
    Nepomuk::IMAccount imAcc2 = cRes2.imAccount();
    Nepomuk::PersonContact pC2 = cRes2.personContact();
    QVERIFY(imAcc2.exists());
    QVERIFY(pC2.exists());

    // Check the default subscription state
    QCOMPARE(imAcc1.publishesPresenceTos().size(), 0);
    QCOMPARE(imAcc2.requestedPresenceSubscriptionTos().size(), 0);

    // Set the subscribe state to Yes
    m_storage->setContactSubscriptionState(QLatin1String("/foo/bar/baz"),
                                           QLatin1String("test@remote-contact.com"),
                                           Tp::Contact::PresenceStateYes);

    // Check
    QCOMPARE(imAcc1.publishesPresenceTos().size(), 1);
    QCOMPARE(imAcc1.publishesPresenceTos().first(), imAcc2);
    QCOMPARE(imAcc2.requestedPresenceSubscriptionTos().size(), 0);

    // Set the subscribe state to Request
    m_storage->setContactSubscriptionState(QLatin1String("/foo/bar/baz"),
                                           QLatin1String("test@remote-contact.com"),
                                           Tp::Contact::PresenceStateAsk);

    // Check
    QCOMPARE(imAcc1.publishesPresenceTos().size(), 0);
    QCOMPARE(imAcc2.requestedPresenceSubscriptionTos().size(), 1);
    QCOMPARE(imAcc2.requestedPresenceSubscriptionTos().first(), imAcc1);

    // Set the subscribe state to no
    m_storage->setContactSubscriptionState(QLatin1String("/foo/bar/baz"),
                                           QLatin1String("test@remote-contact.com"),
                                           Tp::Contact::PresenceStateNo);

    // Check
    QCOMPARE(imAcc1.publishesPresenceTos().size(), 0);
    QCOMPARE(imAcc2.requestedPresenceSubscriptionTos().size(), 0);

    // Cleanup Nepomuk resources used in this test.
    imAcc1.remove();
    imAcc2.remove();
    pC2.remove();
}

void StorageTest::cleanup()
{
    cleanupImpl();

    // Clear re-used member variables.
    if (m_storage) {
        connect(m_storage, SIGNAL(destroyed()), mLoop, SLOT(quit()));
        m_storage->deleteLater();
        mLoop->exec();
        m_storage = 0;
    }
}

void StorageTest::cleanupTestCase()
{
    cleanupTestCaseImpl();
}


QTEST_KDEMAIN(StorageTest, GUI)


#include "storage-test.moc"

