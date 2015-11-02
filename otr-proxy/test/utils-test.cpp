/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public		   *
 * License as published by the Free Software Foundation; either		   *
 * version 2.1 of the License, or (at your option) any later version.	   *
 * 									   *
 * This library is distributed in the hope that it will be useful,	   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU	   *
 * Lesser General Public License for more details.			   *
 * 									   *
 * You should have received a copy of the GNU Lesser General Public	   *
 * License along with this library; if not, write to the Free Software	   *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA*
 ***************************************************************************/

#include "KTpProxy/otr-utils.h"

#include <QtCore>
#include <QtTest>
#include <QtDebug>
#include <QEventLoop>
#include <QMap>
#include <QFile>

class UtilsTest : public QObject
{
    Q_OBJECT;

    private Q_SLOTS:
        void testAccountIdFor();
        void testCmNameFromAccountId();
        void testProtocolFromAccountId();
        void testObjectPathForAccountId();
        void testAccFromAccountId();
};

void UtilsTest::testAccountIdFor()
{
    const QDBusObjectPath path("/org/freedesktop/Telepathy/Account/gabble/jabber/some_account_name_0123");
    QCOMPARE(QLatin1String("gabble.jabber.some_account_name_0123"), OTR::utils::accountIdFor(path));
    QCOMPARE(QLatin1String("gabble.jabber.some_account_name_0123"),
            OTR::utils::accountIdFor(QLatin1String("gabble"),
                QLatin1String("jabber"),
                QLatin1String("some_account_name_0123")));
}

void UtilsTest::testCmNameFromAccountId()
{
    const QLatin1String accountId("gabble.jabber.some_other_account_name_0123");
    QCOMPARE(QLatin1String("gabble"), OTR::utils::cmNameFromAccountId(accountId));
}

void UtilsTest::testProtocolFromAccountId()
{
    const QLatin1String accountId("gabble.jabber.some_other_account_name_0123");
    QCOMPARE(QLatin1String("jabber"), OTR::utils::protocolFromAccountId(accountId));
}

void UtilsTest::testAccFromAccountId()
{
    const QLatin1String accountId("gabble.jabber.some_other_account_name_0123");
    QCOMPARE(QLatin1String("some_other_account_name_0123"), OTR::utils::accFromAccountId(accountId));
}

void UtilsTest::testObjectPathForAccountId()
{
    const QDBusObjectPath path("/org/freedesktop/Telepathy/Account/gabble/jabber/some_account_name_0123");
    QCOMPARE(OTR::utils::objectPathFor(QLatin1String("gabble.jabber.some_account_name_0123")), path);
}

QTEST_MAIN(UtilsTest)
#include "utils-test.moc"
