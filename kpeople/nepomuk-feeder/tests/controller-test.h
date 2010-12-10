/*
 * This file is part of nepomuktelepathyservice
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

#ifndef TELEPATHY_NEPOMUK_SERVICE_CONTROLLER_TEST_H
#define TELEPATHY_NEPOMUK_SERVICE_CONTROLLER_TEST_H

#include "abstract-storage.h"

#include <KTelepathy/TestLib/Test>

#include <TelepathyQt4/AccountManager>

class Controller;

namespace Telepathy {
    class PendingOperation;
}

class ControllerTest : public Test
{
    Q_OBJECT

public:
    ControllerTest(QObject* parent = 0);
    virtual ~ControllerTest();

public Q_SLOTS:
    void constructorDestructorOnAccountManagerReady(Tp::PendingOperation *op);
    void constructorDestructorOnAccountCreated(Tp::PendingOperation *op);
    void constructorDestructorOnAccountCreatedStorage();
    void constructorDestructorOnAccountDestroyedStorage();
    void constructorDestructorOnControllerDestroyed();

private Q_SLOTS:
    void initTestCase();

    void testConstructorDestructor();
  //  void testOnNewAccount();
  //  void testSignalRelays();

    void cleanupTestCase();

private:
    Controller *m_controller;

    bool constructorDestructorAccountDestroyed;

    Tp::AccountManagerPtr m_accountManager;
};

/**
 * Fake subclass of the Storage class that we pass the controller in this unit
 * test to be able to see what slots the controller is calling on it.
 */
class FakeStorage : public AbstractStorage
{
    Q_OBJECT

public:
    FakeStorage(ControllerTest *test);
    virtual ~FakeStorage();

public Q_SLOTS:
    virtual void createAccount(const QString &path, const QString &id, const QString &protocol);
    virtual void destroyAccount(const QString &path);

private:
    ControllerTest *m_test;
};


#endif  // Include guard

