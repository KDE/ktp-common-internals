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

#include "account-test.h"

#include <KTempDir>

#include <Soprano/Backend>
#include <Soprano/PluginManager>
#include <Soprano/StorageModel>
#include <Soprano/QueryResultIterator>

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Thing>
#include <Nepomuk/Variant>

#include "ontologies/nco.h"
#include "ontologies/dataobject.h"
#include "ontologies/informationelement.h"

#include <TelepathyQt4/ConnectionManager>

#include <telepathyaccountmonitor.h>

AccountTest::AccountTest(QObject* parent)
    : TidBaseTest(parent)
{

}
AccountTest::~AccountTest()
{

}

void AccountTest::initTestCase()
{
    initTestCaseImpl();
}

void AccountTest::testSetupAccountMonitor()
{
    setupAccountMonitor();
}

void AccountTest::testAccountCreation()
{
    createAccount();

    // Unfortunately there's no other way: let's wait until the account gets created
    QTimer::singleShot(500, mLoop, SLOT(quit()));
    mLoop->exec();

    // Let's wait for the nepomuk resource to become available
    QString query = QString("select distinct ?a where { %1 %2 ?a . ?a a %3 }")
                            .arg(Soprano::Node::resourceToN3(mePersonContact().resourceUri()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::hasIMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::IMAccount()));

    Soprano::Model *model = Nepomuk::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    // Check that we got some results
    QVERIFY(it.next());
    Nepomuk::IMAccount foundImAccount(it.binding("a").uri());
    // Check for the validity of the resource
    QVERIFY(foundImAccount.isValid());

    // See if the Account has the same Telepathy Account Identifier
    QStringList accountIdentifiers = foundImAccount.accountIdentifiers();
    QCOMPARE(accountIdentifiers.size(), 1);
    QCOMPARE(accountIdentifiers.first(), account()->objectPath());

    // Check if the specified ID matches
    QCOMPARE(foundImAccount.imIDs().first(), QString("foobar"));

    // Check that we got _one_ result
    QVERIFY(!it.next());
}

void AccountTest::testChangeNickname()
{
    // Change the nickname to "Hello KDE"
    Tp::PendingOperation *op = account()->setNickname("Hello KDE");
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(expectSuccessfulCall(Tp::PendingOperation*)));
    QCOMPARE(mLoop->exec(), 0);

    // Unfortunately there's no other way: let's wait until the account gets created
    QTimer::singleShot(500, mLoop, SLOT(quit()));
    mLoop->exec();

    // Let's wait for the nepomuk resource to become available
    QString query = QString("select distinct ?a where { %1 %2 ?a . ?a a %3 }")
                            .arg(Soprano::Node::resourceToN3(mePersonContact().resourceUri()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::hasIMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::IMAccount()));

    Soprano::Model *model = Nepomuk::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    // Check that we got some results
    QVERIFY(it.next());
    Nepomuk::IMAccount foundImAccount(it.binding("a").uri());
    // Check for the validity of the resource
    QVERIFY(foundImAccount.isValid());

    // See if the Account has the same Telepathy Account Identifier
    QStringList accountIdentifiers = foundImAccount.accountIdentifiers();
    QCOMPARE(accountIdentifiers.size(), 1);
    QCOMPARE(accountIdentifiers.first(), account()->objectPath());

    // Check if the nickname matches the new one
    QCOMPARE(foundImAccount.imNicknames().first(), QString("Hello KDE"));

    // Check that we got _one_ result
    QVERIFY(!it.next());
}

void AccountTest::testChangeAvatar()
{
    Tp::Avatar avatar = { QByteArray("adadsdvds"), QLatin1String("image/jpeg") };
    QVERIFY(connect(account()->setAvatar(avatar),
                    SIGNAL(finished(Tp::PendingOperation *)),
                    SLOT(expectSuccessfulCall(Tp::PendingOperation *))));
    QCOMPARE(mLoop->exec(), 0);

    // Unfortunately there's no other way: let's wait until the account gets created
    QTimer::singleShot(500, mLoop, SLOT(quit()));
    mLoop->exec();

    // Let's wait for the nepomuk resource to become available
    QString query = QString("select distinct ?a where { %1 %2 ?a . ?a a %3 }")
                            .arg(Soprano::Node::resourceToN3(mePersonContact().resourceUri()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::hasIMAccount()))
                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NCO::IMAccount()));

    Soprano::Model *model = Nepomuk::ResourceManager::instance()->mainModel();

    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    // Check that we got some results
    QVERIFY(it.next());
    Nepomuk::IMAccount foundImAccount(it.binding("a").uri());
    // Check for the validity of the resource
    QVERIFY(foundImAccount.isValid());

    // See if the Account has the same Telepathy Account Identifier
    QStringList accountIdentifiers = foundImAccount.accountIdentifiers();
    QCOMPARE(accountIdentifiers.size(), 1);
    QCOMPARE(accountIdentifiers.first(), account()->objectPath());

    // Check if the avatar matches the new one
    Nepomuk::Variant vphoto = foundImAccount.property(Nepomuk::Vocabulary::NCO::photo());
    QVERIFY(vphoto.isValid());
    Nepomuk::DataObject photo = vphoto.toResource();

    QCOMPARE(photo.interpretedAses().size(), 1);
    QByteArray imgdata = QByteArray::fromBase64(photo.interpretedAses().first().plainTextContents().first().toUtf8());
    QCOMPARE(QByteArray("adadsdvds"), imgdata);

    // Check that we got _one_ result
    QVERIFY(!it.next());
}

void AccountTest::cleanupTestCase()
{
    cleanupTestCaseImpl();
}

QTEST_MAIN(AccountTest)
#include "account-test.moc"
