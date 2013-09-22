/*
 * This file is part of nepomuktelepathyservice
 *
 * Copyright (C) 2010-2011 Collabora Ltd. <info@collabora.co.uk>
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

#include <TelepathyQt/AccountManager>

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

    void onNewAccountOnAccountManagerReady(Tp::PendingOperation *op);
    void onNewAccountOnAccountCreated(Tp::PendingOperation *op);
    void onNewAccountOnAccountCreatedStorage();

private Q_SLOTS:
    void initTestCase();
    void init();

    void testConstructorDestructor();
    void testOnNewAccount();
  //  void testSignalRelays();

    void cleanup();
    void cleanupTestCase();

private:
    Controller *m_controller;

    bool constructorDestructorAccountDestroyed;

    Tp::AccountManagerPtr m_accountManager;
};

/**
 * Fake subclasses of the Storage class that we pass the controller in this unit
 * test to be able to see what slots the controller is calling on it.
 */
class ConstructorDestructorFakeStorage : public AbstractStorage
{
    Q_OBJECT

public:
    ConstructorDestructorFakeStorage(ControllerTest *test);
    virtual ~ConstructorDestructorFakeStorage();

public Q_SLOTS:
    virtual void createAccount(const QString &path, const QString &id, const QString &protocol);
    virtual void destroyAccount(const QString &path);

    virtual void emitInitialisedSignal();

    // Not used
    virtual void cleanupAccounts(const QList<QString> &) { }
    virtual void setAccountNickname(const QString &, const QString &) { }
    virtual void setAccountCurrentPresence(const QString &, const Tp::SimplePresence &) { }
    virtual void cleanupAccountContacts(const QString &, const QList<QString> &) { }
    virtual void createContact(const QString &, const QString &) { }
    virtual void destroyContact(const QString &, const QString &) { }
    virtual void setContactAlias(const QString &, const QString &, const QString &) { }
    virtual void setContactPresence(const QString &, const QString &, const Tp::SimplePresence &) { }
    virtual void setContactGroups(const QString &, const QString &, const QStringList &) { }
    virtual void setContactBlockStatus(const QString &, const QString &, bool) { }
    virtual void setContactPublishState(const QString &, const QString &, const Tp::Contact::PresenceState &) { }
    virtual void setContactSubscriptionState(const QString &, const QString &, const Tp::Contact::PresenceState &) { }
    virtual void setContactCapabilities(const QString &, const QString &, const Tp::ContactCapabilities &) { }
    virtual void setContactAvatar(const QString &, const QString &, const Tp::AvatarData &) { }

private:
    ControllerTest *m_test;
};

class OnNewAccountFakeStorage : public AbstractStorage
{
    Q_OBJECT

public:
    OnNewAccountFakeStorage(ControllerTest *test);
    virtual ~OnNewAccountFakeStorage();

public Q_SLOTS:
    virtual void createAccount(const QString &path, const QString &id, const QString &protocol);

    virtual void emitInitialisedSignal();

    // Not used
    virtual void cleanupAccounts(const QList<QString> &) { }
    virtual void destroyAccount(const QString &) { }
    virtual void setAccountNickname(const QString &, const QString &) { }
    virtual void setAccountCurrentPresence(const QString &, const Tp::SimplePresence &) { }
    virtual void cleanupAccountContacts(const QString &, const QList<QString> &) { }
    virtual void createContact(const QString &, const QString &) { }
    virtual void destroyContact(const QString &, const QString &) { }
    virtual void setContactAlias(const QString &, const QString &, const QString &) { }
    virtual void setContactPresence(const QString &, const QString &, const Tp::SimplePresence &) { }
    virtual void setContactGroups(const QString &, const QString &, const QStringList &) { }
    virtual void setContactBlockStatus(const QString &, const QString &, bool) { }
    virtual void setContactPublishState(const QString &, const QString &, const Tp::Contact::PresenceState &) { }
    virtual void setContactSubscriptionState(const QString &, const QString &, const Tp::Contact::PresenceState &) { }
    virtual void setContactCapabilities(const QString &, const QString &, const Tp::ContactCapabilities &) { }
    virtual void setContactAvatar(const QString &, const QString &, const Tp::AvatarData &) { }

private:
    ControllerTest *m_test;
};


#endif  // Include guard

