/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2009-2011 Collabora Ltd. <info@collabora.co.uk>
 *   @author Dario Freddi <dario.freddi@collabora.co.uk>
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

#include "account-test.h"

#include "account.h"

#include <KDebug>

#include <qtest_kde.h>

#include <QtTest/QSignalSpy>

#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingReady>

AccountTest::AccountTest(QObject *parent)
: Test(parent),
  m_account(0)
{
    kDebug();
}

AccountTest::~AccountTest()
{
    kDebug();
}

void AccountTest::initTestCase()
{
    initTestCaseImpl();
}

void AccountTest::init()
{
    initImpl();

    // Set up the account manager.
    Tp::Features fAccountFactory;
    fAccountFactory << Tp::Account::FeatureCore
                    << Tp::Account::FeatureAvatar
                    << Tp::Account::FeatureCapabilities
                    << Tp::Account::FeatureProfile
                    << Tp::Account::FeatureProtocolInfo;

    Tp::AccountFactoryConstPtr accountFactory = Tp::AccountFactory::create(
            QDBusConnection::sessionBus(),
            fAccountFactory);

    m_accountManager = Tp::AccountManager::create(accountFactory);

    // Get the Account Manager ready.
    connect(m_accountManager->becomeReady(Tp::Features() << Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            mLoop,
            SLOT(quit()));
    QCOMPARE(mLoop->exec(), 0);
    QVERIFY(m_accountManager->isReady(Tp::Features() << Tp::AccountManager::FeatureCore));

    // Create an account on the account manager
    QVariantMap parameters;
    parameters[QLatin1String("account")] = QLatin1String("baz");
    Tp::PendingAccount *pacc = m_accountManager->createAccount(QLatin1String("foo"),
                                                               QLatin1String("bar"),
                                                               QLatin1String("foobar"),
                                                               parameters);
    connect(pacc,
            SIGNAL(finished(Tp::PendingOperation *)),
            mLoop,
            SLOT(quit()));
    QCOMPARE(mLoop->exec(), 0);

    // Get the account, and check it worked.
    m_account = pacc->account();
    QVERIFY(m_account->isReady(fAccountFactory));
}

void AccountTest::testInitShutdown()
{
    m_accountObject = new Account(m_account, this);

    // Set up spies for the signals we want to watch.
    QSignalSpy spyCreated(m_accountObject,
                          SIGNAL(created(QString,QString,QString)));
    QSignalSpy spyDestroyed(m_accountObject,
                            SIGNAL(accountDestroyed(QString)));
    QSignalSpy spyNicknameChanged(m_accountObject,
                                  SIGNAL(nicknameChanged(QString,QString)));
    QSignalSpy spyCurrentPresenceChanged(m_accountObject,
                                         SIGNAL(currentPresenceChanged(QString, Tp::SimplePresence)));

    // Check the spies are empty at this point.
    QCOMPARE(spyCreated.size(), 0);
    QCOMPARE(spyDestroyed.size(), 0);
    QCOMPARE(spyNicknameChanged.size(), 0);
    QCOMPARE(spyCurrentPresenceChanged.size(), 0);

    // Initialise the account.
    m_accountObject->init();

    // Now check the spies.
    QCOMPARE(spyCreated.size(), 1);
    QCOMPARE(spyDestroyed.size(), 0);
    QCOMPARE(spyNicknameChanged.size(), 1);
    QCOMPARE(spyCurrentPresenceChanged.size(), 1);

    QCOMPARE(spyCreated.first().at(0).toString(), QLatin1String("/org/freedesktop/Telepathy/Account/foo/bar/Account0"));
    QCOMPARE(spyCreated.first().at(1).toString(), QLatin1String("baz"));
    QCOMPARE(spyCreated.first().at(2).toString(), QLatin1String("bar"));
    QCOMPARE(spyNicknameChanged.first().at(0).toString(), QLatin1String("/org/freedesktop/Telepathy/Account/foo/bar/Account0"));
    QCOMPARE(spyNicknameChanged.first().at(1).toString(), QLatin1String("Bob"));
    Tp::SimplePresence p1;
    p1.type = Tp::ConnectionPresenceTypeOffline;
    p1.status = QLatin1String("offline");
    QCOMPARE(spyCurrentPresenceChanged.first().at(0).toString(), QLatin1String("/org/freedesktop/Telepathy/Account/foo/bar/Account0"));
    QCOMPARE(qVariantValue<Tp::SimplePresence>(spyCurrentPresenceChanged.first().at(1)), p1);

    // Clear the spies for the next test.
    spyCreated.clear();
    spyDestroyed.clear();
    spyNicknameChanged.clear();
    spyCurrentPresenceChanged.clear();

    // Now shutdown the account.
    m_accountObject->shutdown();

    // Check the spies.
    QCOMPARE(spyCreated.size(), 0);
    QCOMPARE(spyDestroyed.size(), 1);
    QCOMPARE(spyNicknameChanged.size(), 0);
    QCOMPARE(spyCurrentPresenceChanged.size(), 0);

    QCOMPARE(spyDestroyed.first().at(0).toString(), QLatin1String("/org/freedesktop/Telepathy/Account/foo/bar/Account0"));

    // Clear the spies for the next test
    spyCreated.clear();
    spyDestroyed.clear();
    spyNicknameChanged.clear();
    spyCurrentPresenceChanged.clear();

    // Destroy the account.
    connect(m_accountObject, SIGNAL(destroyed()), mLoop, SLOT(quit()));
    m_accountObject->deleteLater();
    QCOMPARE(mLoop->exec(), 0);
    m_accountObject = 0;

    // Set the nickname and presence of the account to non-default values
    connect(m_account->setNickname(QLatin1String("Ben")),
            SIGNAL(finished(Tp::PendingOperation*)),
            mLoop,
            SLOT(quit()));
    QCOMPARE(mLoop->exec(), 0);
    QCOMPARE(m_account->nickname(), QLatin1String("Ben"));

    Tp::SimplePresence p2;
    p2.status = QLatin1String("available");
    p2.statusMessage = QLatin1String("Test");
    p2.type = Tp::ConnectionPresenceTypeAvailable;
    connect(m_account.data(),
            SIGNAL(currentPresenceChanged(Tp::Presence)),
            mLoop,
            SLOT(quit()));
    m_account->setRequestedPresence(Tp::Presence(p2));
    QCOMPARE(mLoop->exec(), 0);
    QCOMPARE(m_account->currentPresence().barePresence(), p2);

    // Recreate the accountobject
    m_accountObject = new Account(m_account, this);

    // Re-setup the spies
    QSignalSpy spyCreated2(m_accountObject,
                          SIGNAL(created(QString,QString,QString)));
    QSignalSpy spyDestroyed2(m_accountObject,
                            SIGNAL(accountDestroyed(QString)));
    QSignalSpy spyNicknameChanged2(m_accountObject,
                                  SIGNAL(nicknameChanged(QString,QString)));
    QSignalSpy spyCurrentPresenceChanged2(m_accountObject,
                                         SIGNAL(currentPresenceChanged(QString, Tp::SimplePresence)));

    // Init the new accountObject
    m_accountObject->init();

    // Check the spies.
    QCOMPARE(spyCreated2.size(), 1);
    QCOMPARE(spyDestroyed2.size(), 0);
    QCOMPARE(spyNicknameChanged2.size(), 1);
    QCOMPARE(spyCurrentPresenceChanged2.size(), 1);

    QCOMPARE(spyCreated2.first().at(0).toString(), QLatin1String("/org/freedesktop/Telepathy/Account/foo/bar/Account0"));
    QCOMPARE(spyCreated2.first().at(1).toString(), QLatin1String("baz"));
    QCOMPARE(spyCreated2.first().at(2).toString(), QLatin1String("bar"));
    QCOMPARE(spyNicknameChanged2.first().at(0).toString(), QLatin1String("/org/freedesktop/Telepathy/Account/foo/bar/Account0"));
    QCOMPARE(spyNicknameChanged2.first().at(1).toString(), QLatin1String("Ben"));
    QCOMPARE(spyCurrentPresenceChanged2.first().at(0).toString(), QLatin1String("/org/freedesktop/Telepathy/Account/foo/bar/Account0"));
    QCOMPARE(qVariantValue<Tp::SimplePresence>(spyCurrentPresenceChanged2.first().at(1)), p2);
}

void AccountTest::testOnNicknameChanged()
{
    // Set up the account object.
    m_accountObject = new Account(m_account, this);
    m_accountObject->init();

    // Check the default nickname
    QCOMPARE(m_account->nickname(), QLatin1String("Bob"));

    // Change the nickname
    m_account->setNickname(QLatin1String("New Nickname"));

    QSignalSpy spy(m_accountObject, SIGNAL(nicknameChanged(QString,QString)));
    do {
        mLoop->processEvents();
    } while(spy.size() == 0);

    // Check the correct signals were emitted.
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.first().at(1).toString(), QLatin1String("New Nickname"));
}

void AccountTest::testOnCurrentPresenceChanged()
{
    // Set up the account object.
    m_accountObject = new Account(m_account, this);
    m_accountObject->init();

    // Check the default presence
    Tp::SimplePresence p1;
    p1.status = QLatin1String("offline");
    p1.type = Tp::ConnectionPresenceTypeOffline;
    QCOMPARE(m_account->currentPresence().barePresence(), p1);

    // Change the presence
    Tp::SimplePresence p2;
    p2.status = QLatin1String("away");
    p2.statusMessage = QLatin1String("Not Here");
    p2.type = Tp::ConnectionPresenceTypeAway;
    m_account->setRequestedPresence(Tp::Presence(p2));

    QSignalSpy spy(m_accountObject, SIGNAL(currentPresenceChanged(QString,Tp::SimplePresence)));
    do {
        mLoop->processEvents();
    } while(spy.size() == 0);

    // Check the correct signals were emitted.
    QCOMPARE(spy.size(), 1);
    QCOMPARE(qVariantValue<Tp::SimplePresence>(spy.first().at(1)), p2);
}

void AccountTest::cleanup()
{
    cleanupImpl();

    // Remove the account from the AM.
    connect(m_account->remove(),
            SIGNAL(finished(Tp::PendingOperation*)),
            mLoop,
            SLOT(quit()));
    QCOMPARE(mLoop->exec(), 0);

    // Clear TP objects
    m_account.reset();
    m_accountManager.reset();

    // Reset all per-testcase member vars
    if (m_accountObject) {
        connect(m_accountObject, SIGNAL(destroyed()), mLoop, SLOT(quit()));
        m_accountObject->deleteLater();
        mLoop->exec();
        m_accountObject = 0;
    }
}

void AccountTest::cleanupTestCase()
{
    cleanupTestCaseImpl();
}


QTEST_KDEMAIN_CORE(AccountTest)


#include "account-test.moc"

