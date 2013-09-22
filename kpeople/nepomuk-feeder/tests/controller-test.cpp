/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
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

#include "controller-test.h"

#include "controller.h"
#include "abstract-storage.h"

#include <KDebug>

#include <qtest_kde.h>

#include <QtCore/QTimer>

#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/PendingReady>


ConstructorDestructorFakeStorage::ConstructorDestructorFakeStorage(ControllerTest *test)
  : m_test(test)
{
    kDebug();

    QTimer::singleShot(0, this, SIGNAL(emitInitialisedSignal()));
}

ConstructorDestructorFakeStorage::~ConstructorDestructorFakeStorage()
{
    kDebug();
}

void ConstructorDestructorFakeStorage::emitInitialisedSignal()
{
    emit initialised(true);
}

void ConstructorDestructorFakeStorage::createAccount(const QString &path,
                                                     const QString &id,
                                                     const QString &protocol)
{
    Q_UNUSED(path);
    Q_UNUSED(id);
    Q_UNUSED(protocol);
    m_test->constructorDestructorOnAccountCreatedStorage();
}

void ConstructorDestructorFakeStorage::destroyAccount(const QString &path)
{
    Q_UNUSED(path);
    kDebug();
    m_test->constructorDestructorOnAccountDestroyedStorage();
}

OnNewAccountFakeStorage::OnNewAccountFakeStorage(ControllerTest *test)
: m_test(test)
{
    kDebug();

    QTimer::singleShot(0, this, SIGNAL(emitInitialisedSignal()));
}

OnNewAccountFakeStorage::~OnNewAccountFakeStorage()
{
    kDebug();
}


void OnNewAccountFakeStorage::emitInitialisedSignal()
{
    emit initialised(true);
}

void OnNewAccountFakeStorage::createAccount(const QString &path,
                                            const QString &id,
                                            const QString &protocol)
{
    Q_UNUSED(path);
    Q_UNUSED(id);
    Q_UNUSED(protocol);
    m_test->onNewAccountOnAccountCreatedStorage();
}


ControllerTest::ControllerTest(QObject *parent)
  : Test(parent),
    m_controller(0)
{
    kDebug();
}

ControllerTest::~ControllerTest()
{
    kDebug();
}

void ControllerTest::initTestCase()
{
    initTestCaseImpl();
}

void ControllerTest::init()
{
    initImpl();
}

void ControllerTest::testConstructorDestructor()
{
    // First step of this test is to create an account on the AM already.
    m_accountManager = Tp::AccountManager::create();

    connect(m_accountManager->becomeReady(Tp::Features() << Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(constructorDestructorOnAccountManagerReady(Tp::PendingOperation*)));

    QCOMPARE(mLoop->exec(), 1);

    // Now that we have an AM with an Account, we can instantiate the Storage and the Controller
    m_controller = new Controller(new ConstructorDestructorFakeStorage(this));

    QCOMPARE(mLoop->exec(), 2);

    // Next test is to destroy the controller, and check the account is destroyed too.
    constructorDestructorAccountDestroyed = false;
    m_controller->shutdown();

    connect(m_controller, SIGNAL(destroyed()), SLOT(constructorDestructorOnControllerDestroyed()));
    m_controller->deleteLater();
    m_controller = 0;

    QCOMPARE(mLoop->exec(), 3);
    QVERIFY(constructorDestructorAccountDestroyed);
}

void ControllerTest::constructorDestructorOnAccountManagerReady(Tp::PendingOperation *op)
{
    QVERIFY(!op->isError());

    // Now the AM is ready, create an Account on it.
    connect(m_accountManager->createAccount("test", "test", "test", QVariantMap()),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(constructorDestructorOnAccountCreated(Tp::PendingOperation*)));
}

void ControllerTest::constructorDestructorOnAccountCreated(Tp::PendingOperation *op)
{
    QVERIFY(!op->isError());

    // Done. Exit event loop.
    mLoop->exit(1);
}

void ControllerTest::constructorDestructorOnAccountCreatedStorage()
{
    mLoop->exit(2);
}

void ControllerTest::constructorDestructorOnAccountDestroyedStorage()
{
    kDebug();
    constructorDestructorAccountDestroyed = true;
}

void ControllerTest::constructorDestructorOnControllerDestroyed()
{
    mLoop->exit(3);
}

void ControllerTest::testOnNewAccount()
{
    // Get an AM and get it ready.
    m_accountManager = Tp::AccountManager::create();

    connect(m_accountManager->becomeReady(Tp::Features() << Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onNewAccountOnAccountManagerReady(Tp::PendingOperation*)));

    QCOMPARE(mLoop->exec(), 1);

    // Set up the Controller and Fake Storage
    m_controller = new Controller(new OnNewAccountFakeStorage(this));

    QVERIFY(m_controller);

    // Add a new Account to the AM.
    connect(m_accountManager->createAccount("test", "test", "test", QVariantMap()),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onNewAccountOnAccountCreated(Tp::PendingOperation*)));

    QCOMPARE(mLoop->exec(), 2);
}

void ControllerTest::onNewAccountOnAccountManagerReady(Tp::PendingOperation *op)
{
    QVERIFY(!op->isError());

    mLoop->exit(1);
}

void ControllerTest::onNewAccountOnAccountCreated(Tp::PendingOperation *op)
{
    // Check that this actually succeeded.
    QVERIFY(!op->isError());

    // If it failed, exit the event loop.
    if (op->isError()) {
        mLoop->exit(0);
    }
}

void ControllerTest::onNewAccountOnAccountCreatedStorage()
{
    mLoop->exit(2);
}

void ControllerTest::cleanup()
{
    cleanupImpl();

    // Clear re-used member variables.
    if (m_controller) {
        connect(m_controller, SIGNAL(destroyed()), mLoop, SLOT(quit()));
        m_controller->deleteLater();
        mLoop->exec();
        m_controller = 0;
    }

    m_accountManager.reset();
}

void ControllerTest::cleanupTestCase()
{
    cleanupTestCaseImpl();
}


QTEST_KDEMAIN_CORE(ControllerTest)


#include "controller-test.moc"

