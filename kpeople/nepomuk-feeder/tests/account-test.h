/*
 * This file is part of telepathy-integration-daemon
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

#ifndef ACCOUNT_TEST_H
#define ACCOUNT_TEST_H

#include "tid-base-test.h"

#include <TelepathyQt4/Types>

#include <Nepomuk/Resource>

class TelepathyAccountMonitor;
namespace Soprano {
class StorageModel;
}

class KTempDir;
class AccountTest : public TidBaseTest
{
    Q_OBJECT
public:
    AccountTest(QObject* parent = 0);
    virtual ~AccountTest();

private Q_SLOTS:
    void initTestCase();

    void testSetupAccountMonitor();
    void testAccountCreation();
    void testChangeNickname();
    void testChangeAvatar();

    void cleanupTestCase();
};

#endif // ACCOUNT_TEST_H
