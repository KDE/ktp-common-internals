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

#include "tid-base-test.h"

#include <Soprano/Backend>
#include <Soprano/StorageModel>
#include <Soprano/PluginManager>

#include <Nepomuk2/ResourceManager>
#include <Nepomuk2/Thing>

#include "ontologies/nco.h"

#include <TelepathyQt4/PendingAccount>
#include <TelepathyQt4/PendingReady>

#include <KTempDir>

#include <telepathyaccountmonitor.h>

class TidBaseTest::Private
{
    Q_DECLARE_PUBLIC(TidBaseTest)
    TidBaseTest * const q_ptr;
    public:
        Private(TidBaseTest *parent) : q_ptr(parent) {}
        virtual ~Private() {}

        TelepathyAccountMonitor *monitor;
        Nepomuk2::Resource mePersonContact;
        Tp::AccountPtr account;
        Tp::ConnectionPtr connection;
};

TidBaseTest::TidBaseTest(QObject* parent)
    : NepomukTest(parent)
    , d(new Private(this))
{
}

TidBaseTest::~TidBaseTest()
{
    delete d;
}

void TidBaseTest::initTestCaseImpl()
{
    Tp::NepomukTest::initTestCaseImpl();
}

void TidBaseTest::initTestCaseConnectionImpl()
{
    initTestCaseImpl();

    setupExampleConnection("account", QLatin1String("foobar"), "protocol", "contactlist");

    d->connection = Tp::Connection::create(exampleConnectionData().first, exampleConnectionData().second);

    QVERIFY(connect(d->connection->requestConnect(),
                    SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(expectSuccessfulCall(Tp::PendingOperation*))));
    QCOMPARE(mLoop->exec(), 0);
    QCOMPARE(d->connection->isReady(), true);
    qDebug() << d->connection->status();
    QCOMPARE(d->connection->status(), Tp::Connection::StatusConnected);
}

void TidBaseTest::cleanupTestCaseConnectionImpl()
{
    if (d->connection) {
        // Disconnect and wait for the readiness change
        QVERIFY(connect(d->connection->requestDisconnect(),
                        SIGNAL(finished(Tp::PendingOperation*)),
                        SLOT(expectSuccessfulCall(Tp::PendingOperation*))));
        QCOMPARE(mLoop->exec(), 0);

        if (d->connection->isValid()) {
            QVERIFY(connect(d->connection.data(),
                            SIGNAL(invalidated(Tp::DBusProxy *,
                                               const QString &, const QString &)),
                            mLoop,
                            SLOT(quit())));
            QCOMPARE(mLoop->exec(), 0);
        }
    }

    cleanupTestCaseImpl();
}

void TidBaseTest::cleanupTestCaseImpl()
{
    Tp::NepomukTest::cleanupTestCaseImpl();
}
void TidBaseTest::setupAccountMonitor()
{
    d->monitor = new TelepathyAccountMonitor();

    // At this very point, we should have a "me" PIMO person up and running
    // FIXME: Port to new OSCAF standard for accessing "me" as soon as it
    // becomes available.
    Nepomuk2::Thing me(QUrl::fromEncoded("nepomuk:/me"));

    QVERIFY(me.exists());
    // Loop through all the grounding instances of this person
    Q_FOREACH (Nepomuk2::Resource resource, me.groundingOccurrences()) {
        // See if this grounding instance is of type nco:contact.
        if (resource.hasType(Nepomuk2::Vocabulary::NCO::PersonContact())) {
            // FIXME: We are going to assume the first NCO::PersonContact is the
            // right one. Can we improve this?
            d->mePersonContact = resource;
            break;
        }
    }
    QVERIFY(d->mePersonContact.isValid());
    QVERIFY(me.groundingOccurrences().contains(d->mePersonContact));
}

TelepathyAccountMonitor* TidBaseTest::accountMonitor()
{
    return d->monitor;
}

Nepomuk2::Resource TidBaseTest::mePersonContact()
{
    return d->mePersonContact;
}

void TidBaseTest::createAccount()
{
    QVariantMap parameters;
    parameters[QLatin1String("account")] = QLatin1String("foobar");
    Tp::PendingAccount *pacc = accountMonitor()->accountManager()->createAccount(QLatin1String("foo"),
                                              QLatin1String("bar"), QLatin1String("foobar"), parameters);
    QVERIFY(connect(pacc,
                    SIGNAL(finished(Tp::PendingOperation *)),
                    SLOT(expectSuccessfulCall(Tp::PendingOperation *))));
    QCOMPARE(mLoop->exec(), 0);
    d->account = pacc->account();
    QVERIFY(d->account);
}

Tp::AccountPtr TidBaseTest::account()
{
    return d->account;
}

Tp::ConnectionPtr TidBaseTest::connection()
{
    return d->connection;
}

#include "tid-base-test.moc"
