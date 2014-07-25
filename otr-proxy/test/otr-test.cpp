/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "lib/test-config.h"
#include "lib/test-session.h"

#include <KTpProxy/otr-manager.h>

#include <QtCore>
#include <QtTest>
#include <QtDebug>
#include <QEventLoop>
#include <QMap>
#include <QFile>

extern "C" {
#include <libotr/privkey.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
}

using namespace OTR;

namespace tst {

    const SessionContext aliceCtx =
    {
        QLatin1String("alice_id"),
        QLatin1String("Alice"),
        QLatin1String("Bob"),
        QLatin1String("talk")
    };

    const SessionContext bobCtx =
    {
        QLatin1String("bob_id"),
        QLatin1String("Bob"),
        QLatin1String("Alice"),
        QLatin1String("talk")
    };

    class SessionEnv : public QObject
    {
        Q_OBJECT;

        public:
            SessionEnv(const SessionContext &ctx, Manager *manager)
                : ses(ctx, manager)
            {
            }

            void reset()
            {
                lastFpRcv.clear();
                sessionRefreshed = false;
                lastTlevel = TrustLevel::NOT_PRIVATE;
            }

        public Q_SLOTS:
            void onNewFingerprintReceived(const QString &fp)
            {
                lastFpRcv = fp;
            }
            void onSessionRefreshed()
            {
                sessionRefreshed = true;
            }

            void onTrustLevelChanged(TrustLevel tlevel)
            {
                lastTlevel = tlevel;
            }

        public:
            TestSession ses;
            QString lastFpRcv;
            bool sessionRefreshed;
            TrustLevel lastTlevel;
    };

} /* namespace tst */


class OTRTest : public QObject
{
    Q_OBJECT

    public:
        OTRTest();

    private Q_SLOTS:
        void initTestCase();
        void testSimpleSession();

        void cleanup();

    private:
        TestConfig config;
        Manager manager;
};

OTRTest::OTRTest()
: manager(&config)
{
}

void OTRTest::initTestCase()
{
    OTRL_INIT;
}

void OTRTest::testSimpleSession()
{
    tst::SessionEnv aliceEnv(tst::aliceCtx, &manager);
    tst::SessionEnv bobEnv(tst::bobCtx, &manager);
    TestSession &alice = aliceEnv.ses;
    TestSession &bob = bobEnv.ses;

    QVERIFY(connect(&alice, SIGNAL(newFingeprintReceived(const QString&)),
                &aliceEnv, SLOT(onNewFingerprintReceived(const QString&))));
    QVERIFY(connect(&bob, SIGNAL(newFingeprintReceived(const QString&)),
                &bobEnv, SLOT(onNewFingerprintReceived(const QString&))));

    Message mes = alice.startSession();
    QCOMPARE(mes.direction(), MessageDirection::TO_PEER);
    QVERIFY(alice.mesQueue.empty());

    QCOMPARE(bob.decrypt(mes), CryptResult::OTR);
    QVERIFY(!bob.mesQueue.empty());
    // session initialization
    while(!bob.mesQueue.empty() || !alice.mesQueue.empty()) {
        if(!bob.mesQueue.empty()) {
            QCOMPARE(bob.mesQueue.back().direction(), MessageDirection::TO_PEER);
            QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
            bob.mesQueue.pop_back();
        }
        if(!alice.mesQueue.empty()) {
            QCOMPARE(alice.mesQueue.back().direction(), MessageDirection::TO_PEER);
            QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
            alice.mesQueue.pop_back();
        }
    }
    QVERIFY(alice.mesQueue.empty());
    QVERIFY(alice.eventQueue.empty());
    QVERIFY(bob.mesQueue.empty());
    QVERIFY(bob.eventQueue.empty());

    QVERIFY(!bobEnv.lastFpRcv.isEmpty());
    QVERIFY(!aliceEnv.lastFpRcv.isEmpty());

    QCOMPARE(bob.trustLevel(), TrustLevel::UNVERIFIED);
    QCOMPARE(alice.trustLevel(), TrustLevel::UNVERIFIED);
    // now the session is instantiated
    // everything should be encrypted from now on

    Message helloMsg;
    const QString helloText = QLatin1String("No witej chopie!");
    helloMsg.setText(helloText);
    QCOMPARE(bob.encrypt(helloMsg), CryptResult::CHANGED);
    QVERIFY(bob.eventQueue.empty());

    QCOMPARE(alice.decrypt(helloMsg), CryptResult::CHANGED);
    QVERIFY(alice.eventQueue.empty());
    QCOMPARE(helloMsg.text(), helloText);

    Message responseMsg;
    const QString responseText = QLatin1String("Siema leszczu!");
    responseMsg.setText(responseText);
    QCOMPARE(alice.encrypt(responseMsg), CryptResult::CHANGED);
    QVERIFY(alice.eventQueue.empty());

    QCOMPARE(bob.decrypt(responseMsg), CryptResult::CHANGED);
    QVERIFY(bob.eventQueue.empty());
    QCOMPARE(responseMsg.text(), responseText);

    bob.stopSession();
    QVERIFY(!bob.mesQueue.empty());
    QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
    bob.mesQueue.pop_back();
    QCOMPARE(bob.trustLevel(), TrustLevel::NOT_PRIVATE);
    QCOMPARE(alice.trustLevel(), TrustLevel::FINISHED);
    QVERIFY(alice.eventQueue.empty());
    QVERIFY(bob.eventQueue.empty());
}

void OTRTest::cleanup()
{
    QFile(config.saveLocation() + tst::aliceCtx.accountId + QLatin1String("_fingerprints")).remove();
    QFile(config.saveLocation() + tst::bobCtx.accountId + QLatin1String("_fingerprints")).remove();
    manager.setPolicy(OTRL_POLICY_MANUAL); // default policy
}


QTEST_MAIN(OTRTest)
#include "otr-test.moc"
