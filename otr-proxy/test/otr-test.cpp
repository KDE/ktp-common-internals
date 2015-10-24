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

#include "lib/test-config.h"
#include "lib/test-session.h"

#include "KTp/OTR/types.h"

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

    const SessionContext aliceJCtx =
    {
        QLatin1String("alice_id"),
        QLatin1String("Alice"),
        QLatin1String("John"),
        QLatin1String("talk")
    };
    const SessionContext johnCtx =
    {
        QLatin1String("john_id"),
        QLatin1String("John"),
        QLatin1String("Alice"),
        QLatin1String("talk")
    };


    class SessionEnv : public QObject
    {
        Q_OBJECT;

        public:
            SessionEnv(const SessionContext &ctx, Manager *manager)
                : ses(ctx, manager),
                sessionRefreshed(false),
                levelChanged(false),
                smpAborted(false),
                smpFinished(false),
                smpSuccess(false),
                smpQuery(false),
                question(QLatin1String(""))
            {
                QVERIFY(connect(&ses, SIGNAL(sessionRefreshed()), SLOT(onSessionRefreshed())));
                QVERIFY(connect(&ses, SIGNAL(trustLevelChanged(TrustLevel)), SLOT(onTrustLevelChanged(TrustLevel))));
                QVERIFY(connect(&ses, SIGNAL(authenticationConcluded(bool)), SLOT(onSMPConcluded(bool))));
                QVERIFY(connect(&ses, SIGNAL(authenticationAborted()), SLOT(onSMPAborted())));
                QVERIFY(connect(&ses, SIGNAL(authenticationRequested(const QString&)), SLOT(onSMPQuery(const QString&))));
            }

            void reset()
            {
                lastFpRcv.clear();
                sessionRefreshed = false;
                levelChanged = false;
                lastTlevel = TrustLevel::NOT_PRIVATE;

                smpAborted = false;
                smpFinished = false;
                smpSuccess = false;
                smpQuery = false;
                question.clear();

                ses.eventQueue.clear();
                ses.mesQueue.clear();
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
                levelChanged = true;
                lastTlevel = tlevel;
            }

            void onSMPConcluded(bool success)
            {
                smpFinished = true;
                smpSuccess = success;
            }

            void onSMPAborted()
            {
                smpAborted = true;
            }

            void onSMPQuery(const QString &question)
            {
                smpQuery = true;
                this->question = question;
            }

        public:
            TestSession ses;
            QString lastFpRcv;
            bool sessionRefreshed;
            bool levelChanged;
            TrustLevel lastTlevel;

            bool smpAborted;
            bool smpFinished;
            bool smpSuccess;
            bool smpQuery;
            QString question;
    };

} /* namespace tst */


class OTRTest : public QObject
{
    Q_OBJECT

    public:
        OTRTest();

    private Q_SLOTS:
        void initTestCase();
        void init();

        void testSimpleSession();
        void testSessionPolicyNever();
        void testSessionPolicyOpportunistic();
        void testSessionPolicyAlways();
        void testEncUnencryptedErrors();
        void testDoubleConversation();
        void testTrustDistrustFingerprint();
        void testSessionRefreshed();
        void testForceUnencrypted();
        void testSMPQuerySuccess();
        void testSMPSecretSuccess();
        void testSMPQueryFail();
        void testSMPQueryFailDistrust();
        void testSMPSecretFailDistrust();
        void testSMPQueryAborted();
        void testFingerprintManagement();
        void testDiffrentContentTypes();

        void cleanup();

    private:

        void startSession(TestSession& alice, TestSession &bob);
        void stopSession(TestSession& alice, TestSession &bob);

        TestConfig aliceConfig;
        TestConfig bobConfig;
        Manager *aliceMan;
        Manager *bobMan;
        tst::SessionEnv *aliceEnv;
        tst::SessionEnv *bobEnv;
};

OTRTest::OTRTest()
: aliceMan(nullptr),
    bobMan(nullptr),
    aliceEnv(nullptr),
    bobEnv(nullptr)
{
}

void OTRTest::initTestCase()
{
    OTRL_INIT;
}

void OTRTest::init()
{
    aliceMan = new Manager(&aliceConfig);
    bobMan = new Manager(&bobConfig);
    aliceEnv = new tst::SessionEnv(tst::aliceCtx, aliceMan);
    bobEnv = new tst::SessionEnv(tst::bobCtx, bobMan);
}

void OTRTest::testSimpleSession()
{
    aliceMan->setPolicy(OTRL_POLICY_MANUAL);
    bobMan->setPolicy(OTRL_POLICY_MANUAL);
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    QVERIFY(connect(&alice, SIGNAL(newFingerprintReceived(const QString&)),
                aliceEnv, SLOT(onNewFingerprintReceived(const QString&))));
    QVERIFY(connect(&bob, SIGNAL(newFingerprintReceived(const QString&)),
                bobEnv, SLOT(onNewFingerprintReceived(const QString&))));

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

    QVERIFY(!bobEnv->lastFpRcv.isEmpty());
    QVERIFY(!aliceEnv->lastFpRcv.isEmpty());

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

void OTRTest::testSessionPolicyNever()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    bobMan->setPolicy(OTRL_POLICY_NEVER);

    Message initMes = alice.startSession();
    QCOMPARE(bob.decrypt(initMes), CryptResult::UNCHANGED);
    QVERIFY(bob.mesQueue.empty());
    QVERIFY(bob.eventQueue.empty());

    Message helloMsg;
    const QString heyText = QLatin1String("Hey Alice");
    helloMsg.setText(heyText);
    QCOMPARE(alice.decrypt(helloMsg), CryptResult::UNCHANGED);
    QCOMPARE(helloMsg.text(), heyText);

    QCOMPARE(alice.trustLevel(), TrustLevel::NOT_PRIVATE);
    QCOMPARE(bob.trustLevel(), TrustLevel::NOT_PRIVATE);
}

void OTRTest::testSessionPolicyOpportunistic()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    aliceMan->setPolicy(OTRL_POLICY_MANUAL);
    bobMan->setPolicy(OTRL_POLICY_OPPORTUNISTIC);
    Message helloMsg;
    const QString heyText = QLatin1String("Hey Alice");
    helloMsg.setText(heyText);
    QCOMPARE(bob.encrypt(helloMsg), CryptResult::CHANGED);
    QVERIFY(bob.mesQueue.empty());
    QVERIFY(bob.eventQueue.empty());

    QCOMPARE(alice.decrypt(helloMsg), CryptResult::CHANGED);
    QVERIFY(alice.mesQueue.empty());
    QVERIFY(alice.eventQueue.empty());

    // now alice has also opportunistic policy
    QCOMPARE(bob.encrypt(helloMsg), CryptResult::CHANGED);
    aliceMan->setPolicy(OTRL_POLICY_OPPORTUNISTIC);
    QCOMPARE(alice.decrypt(helloMsg), CryptResult::CHANGED);
    QVERIFY(!alice.mesQueue.empty());
    QVERIFY(alice.eventQueue.empty());

    while(!bob.mesQueue.empty() || !alice.mesQueue.empty()) {
        if(!alice.mesQueue.empty()) {
            QCOMPARE(alice.mesQueue.back().direction(), MessageDirection::TO_PEER);
            QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
            alice.mesQueue.pop_back();
        }
        if(!bob.mesQueue.empty()) {
            QCOMPARE(bob.mesQueue.back().direction(), MessageDirection::TO_PEER);
            QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
            bob.mesQueue.pop_back();
        }
    }

    QCOMPARE(bob.trustLevel(), TrustLevel::UNVERIFIED);
    QCOMPARE(alice.trustLevel(), TrustLevel::UNVERIFIED);
}

void OTRTest::testSessionPolicyAlways()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    aliceMan->setPolicy(OTRL_POLICY_ALWAYS);
    Message helloMsg;
    const QString heyText = QLatin1String("Hey Alice");
    helloMsg.setText(heyText);

    QCOMPARE(alice.decrypt(helloMsg), CryptResult::OTR);
    QVERIFY(alice.mesQueue.empty());
    QVERIFY(!alice.eventQueue.empty());
    QCOMPARE(alice.eventQueue.back().getOTRheader(QLatin1String("otr-unencrypted-message")), heyText);
    QCOMPARE(alice.eventQueue.back().getOTRevent(), OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED);

    aliceEnv->reset();
    bobEnv->reset();

    bobMan->setPolicy(OTRL_POLICY_OPPORTUNISTIC);
    helloMsg.setText(heyText);

    // now the same as at opportunistic
    Message initMsg = bob.startSession();

    QCOMPARE(alice.decrypt(initMsg), CryptResult::OTR);
    QVERIFY(!alice.mesQueue.empty());
    QVERIFY(alice.eventQueue.empty());

    while(!bob.mesQueue.empty() || !alice.mesQueue.empty()) {
        if(!alice.mesQueue.empty()) {
            QCOMPARE(alice.mesQueue.back().direction(), MessageDirection::TO_PEER);
            QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
            alice.mesQueue.pop_back();
        }
        if(!bob.mesQueue.empty()) {
            QCOMPARE(bob.mesQueue.back().direction(), MessageDirection::TO_PEER);
            QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
            bob.mesQueue.pop_back();
        }
    }

    QCOMPARE(bob.trustLevel(), TrustLevel::UNVERIFIED);
    QCOMPARE(alice.trustLevel(), TrustLevel::UNVERIFIED);
}

void OTRTest::startSession(TestSession &alice, TestSession &bob)
{
    Message mes = alice.startSession();
    QCOMPARE(mes.direction(), MessageDirection::TO_PEER);
    QVERIFY(alice.mesQueue.empty());

    QCOMPARE(bob.decrypt(mes), CryptResult::OTR);
    QVERIFY(!bob.mesQueue.empty());
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
}

void OTRTest::stopSession(TestSession &alice, TestSession &bob)
{
    alice.stopSession();
    QVERIFY(!alice.mesQueue.empty());

    while(!bob.mesQueue.empty() || !alice.mesQueue.empty()) {
        if(!alice.mesQueue.empty()) {
            QCOMPARE(alice.mesQueue.back().direction(), MessageDirection::TO_PEER);
            QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
            alice.mesQueue.pop_back();
        }
        if(!bob.mesQueue.empty()) {
            QCOMPARE(bob.mesQueue.back().direction(), MessageDirection::TO_PEER);
            QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
            bob.mesQueue.pop_back();
        }
    }
    QVERIFY(alice.mesQueue.empty());
    QVERIFY(alice.eventQueue.empty());
    QVERIFY(bob.mesQueue.empty());
    QVERIFY(bob.eventQueue.empty());
}

void OTRTest::testEncUnencryptedErrors()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;
    startSession(alice, bob);

    Message mes;
    const QString text = QLatin1String("Unencrypted text");
    mes.setText(text);

    QCOMPARE(alice.decrypt(mes), CryptResult::OTR);
    QVERIFY(alice.mesQueue.empty());
    QVERIFY(!alice.eventQueue.empty());

    Message eventMsg = alice.eventQueue.back();
    alice.eventQueue.clear();

    QCOMPARE(eventMsg.getOTRevent(), OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED);
    QCOMPARE(eventMsg.getOTRheader(QLatin1String("otr-unencrypted-message")), text);

    mes.setText(text);
    QCOMPARE(bob.encrypt(mes), CryptResult::CHANGED);

    alice.mesQueue.clear();
    alice.stopSession();
    QCOMPARE(alice.trustLevel(), TrustLevel::NOT_PRIVATE);
    QCOMPARE(alice.decrypt(mes), CryptResult::OTR);
    QVERIFY(!alice.eventQueue.empty());
    QVERIFY(!alice.mesQueue.empty());

    eventMsg = alice.eventQueue.back();
    QCOMPARE(eventMsg.getOTRevent(), OTRL_MSGEVENT_RCVDMSG_NOT_IN_PRIVATE);

    QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
    QVERIFY(bob.mesQueue.empty());
    QVERIFY(bob.eventQueue.empty());
    QCOMPARE(bob.trustLevel(), TrustLevel::FINISHED);
}

void OTRTest::testDoubleConversation()
{
    tst::SessionEnv johnEnv(tst::johnCtx, bobMan);
    tst::SessionEnv aliceJEnv(tst::aliceJCtx, aliceMan);

    TestSession &alice = aliceEnv->ses;
    TestSession &aliceJ = aliceJEnv.ses;
    TestSession &bob = bobEnv->ses;
    TestSession &john = johnEnv.ses;

    startSession(alice, bob);
    startSession(aliceJ, john);

    Message aliceToBob;
    const QString aliceToBobText = QLatin1String("Hey Bob");
    aliceToBob.setText(aliceToBobText);
    QCOMPARE(alice.encrypt(aliceToBob), CryptResult::CHANGED);

    Message aliceToJohn;
    const QString aliceToJohnText = QLatin1String("Hello John");
    aliceToJohn.setText(aliceToJohnText);
    QCOMPARE(aliceJ.encrypt(aliceToJohn), CryptResult::CHANGED);

    QCOMPARE(bob.decrypt(aliceToBob), CryptResult::CHANGED);
    QCOMPARE(aliceToBob.text(), aliceToBobText);
    QCOMPARE(john.decrypt(aliceToJohn), CryptResult::CHANGED);
    QCOMPARE(aliceToJohn.text(), aliceToJohnText);

    bob.stopSession();
    QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
    // john with alice still should have encrypted conversation

    Message johnToAlice;
    const QString johnToAliceText = QLatin1String("Hi Alice!!!");
    johnToAlice.setText(johnToAliceText);
    QCOMPARE(john.encrypt(johnToAlice), CryptResult::CHANGED);
    QCOMPARE(aliceJ.decrypt(johnToAlice), CryptResult::CHANGED);
    QCOMPARE(johnToAlice.text(), johnToAliceText);

    QCOMPARE(aliceJ.trustLevel(), TrustLevel::UNVERIFIED);
    QCOMPARE(john.trustLevel(), TrustLevel::UNVERIFIED);
    QCOMPARE(bob.trustLevel(), TrustLevel::NOT_PRIVATE);
    QCOMPARE(alice.trustLevel(), TrustLevel::FINISHED);
}

void OTRTest::testTrustDistrustFingerprint()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    QCOMPARE(TrustFpResult::NO_SUCH_FINGERPRINT, alice.trustFingerprint(true));

    startSession(alice, bob);
    QCOMPARE(alice.trustLevel(), TrustLevel::UNVERIFIED);
    QCOMPARE(bob.trustLevel(), TrustLevel::UNVERIFIED);

    QCOMPARE(TrustFpResult::OK, alice.trustFingerprint(true));
    QCOMPARE(alice.trustLevel(), TrustLevel::VERIFIED);

    delete aliceEnv;
    delete bobEnv;
    delete bobMan;
    delete aliceMan;

    init();

    TestSession &alice2 = aliceEnv->ses;
    TestSession &bob2 = bobEnv->ses;
    startSession(alice2, bob2);
    QCOMPARE(alice2.trustLevel(), TrustLevel::VERIFIED);
    QCOMPARE(bob2.trustLevel(), TrustLevel::UNVERIFIED);

    QCOMPARE(TrustFpResult::OK, alice2.trustFingerprint(false));
    QCOMPARE(alice2.trustLevel(), TrustLevel::UNVERIFIED);
    QCOMPARE(bob2.trustLevel(), TrustLevel::UNVERIFIED);

    delete aliceEnv;
    delete bobEnv;
    delete bobMan;
    delete aliceMan;
    init();

    TestSession &alice3 = aliceEnv->ses;
    TestSession &bob3 = bobEnv->ses;
    startSession(alice3, bob3);
    QCOMPARE(alice3.trustLevel(), TrustLevel::UNVERIFIED);
    QCOMPARE(bob3.trustLevel(), TrustLevel::UNVERIFIED);
}

void OTRTest::testSessionRefreshed()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    startSession(alice, bob);
    startSession(alice, bob);
    QVERIFY(aliceEnv->sessionRefreshed);
    QVERIFY(bobEnv->levelChanged);
}

void OTRTest::testForceUnencrypted()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    startSession(alice, bob);
    alice.forceUnencrypted();
    QCOMPARE(alice.trustLevel(), TrustLevel::NOT_PRIVATE);
}

void OTRTest::testSMPQuerySuccess()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    startSession(alice, bob);

    QVERIFY(alice.mesQueue.isEmpty());
    QVERIFY(bob.mesQueue.isEmpty());

    const QString question = QLatin1String("Best polish grindcore band?");
    const QString answer = QLatin1String("Antigama");
    alice.initSMPQuery(question, answer);
    QVERIFY(!alice.mesQueue.isEmpty());

    QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
    alice.mesQueue.clear();
    QVERIFY(bobEnv->smpQuery);
    QVERIFY(bob.mesQueue.empty());

    QCOMPARE(bobEnv->question, question);

    bob.respondSMPAuthentication(answer);
    QVERIFY(!bob.mesQueue.isEmpty());

    QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
    bob.mesQueue.clear();
    QVERIFY(!alice.mesQueue.empty());
    QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
    alice.mesQueue.clear();

    QVERIFY(!bob.mesQueue.isEmpty());
    QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
    QVERIFY(aliceEnv->smpFinished);
    QVERIFY(bobEnv->smpFinished);
    QVERIFY(aliceEnv->smpSuccess);
    QVERIFY(bobEnv->smpSuccess);
    QVERIFY(alice.mesQueue.empty());

    QCOMPARE(alice.trustLevel(), TrustLevel::VERIFIED);
    QCOMPARE(bob.trustLevel(), TrustLevel::UNVERIFIED);
    QVERIFY(aliceEnv->levelChanged);
}

void OTRTest::testSMPSecretSuccess()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    startSession(alice, bob);

    QVERIFY(alice.mesQueue.isEmpty());
    QVERIFY(bob.mesQueue.isEmpty());

    const QString answer = QLatin1String("Bigos");
    alice.initSMPSecret(answer);
    QVERIFY(!alice.mesQueue.isEmpty());

    QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
    alice.mesQueue.clear();
    QVERIFY(bobEnv->smpQuery);
    QVERIFY(bob.mesQueue.empty());

    QVERIFY(bobEnv->question.isEmpty());

    bob.respondSMPAuthentication(answer);
    QVERIFY(!bob.mesQueue.isEmpty());

    QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
    bob.mesQueue.clear();
    QVERIFY(!alice.mesQueue.empty());
    QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
    alice.mesQueue.clear();

    QVERIFY(!bob.mesQueue.isEmpty());
    QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
    QVERIFY(aliceEnv->smpFinished);
    QVERIFY(bobEnv->smpFinished);
    QVERIFY(aliceEnv->smpSuccess);
    QVERIFY(bobEnv->smpSuccess);
    QVERIFY(alice.mesQueue.empty());

    QCOMPARE(alice.trustLevel(), TrustLevel::VERIFIED);
    QCOMPARE(bob.trustLevel(), TrustLevel::VERIFIED);
    QVERIFY(aliceEnv->levelChanged);
    QVERIFY(bobEnv->levelChanged);
}

void OTRTest::testSMPQueryFail()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    startSession(alice, bob);

    const QString question = QLatin1String("Best polish grindcore band?");
    const QString answer = QLatin1String("Antigama");
    alice.initSMPQuery(question, answer);
    QVERIFY(!alice.mesQueue.isEmpty());

    QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
    alice.mesQueue.clear();
    QVERIFY(bobEnv->smpQuery);
    QVERIFY(bob.mesQueue.empty());

    QCOMPARE(bobEnv->question, question);

    bob.respondSMPAuthentication(QLatin1String("Doda"));
    QVERIFY(!bob.mesQueue.isEmpty());

    QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
    bob.mesQueue.clear();
    QVERIFY(!alice.mesQueue.empty());
    QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
    alice.mesQueue.clear();

    QVERIFY(!bob.mesQueue.isEmpty());
    QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
    QVERIFY(aliceEnv->smpFinished);
    QVERIFY(bobEnv->smpFinished);
    QVERIFY(!aliceEnv->smpSuccess);
    QVERIFY(!bobEnv->smpSuccess);
    QVERIFY(alice.mesQueue.empty());

    QCOMPARE(alice.trustLevel(), TrustLevel::UNVERIFIED);
    QCOMPARE(bob.trustLevel(), TrustLevel::UNVERIFIED);
    QVERIFY(aliceEnv->levelChanged);
    QVERIFY(bobEnv->levelChanged);
}

void OTRTest::testSMPQueryFailDistrust()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    startSession(alice, bob);

    alice.trustFingerprint(true);
    QCOMPARE(alice.trustLevel(), TrustLevel::VERIFIED);
    testSMPQueryFail();
}

void OTRTest::testSMPSecretFailDistrust()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    startSession(alice, bob);

    alice.trustFingerprint(true);
    bob.trustFingerprint(true);
    QCOMPARE(alice.trustLevel(), TrustLevel::VERIFIED);
    QCOMPARE(bob.trustLevel(), TrustLevel::VERIFIED);

    QVERIFY(alice.mesQueue.isEmpty());
    QVERIFY(bob.mesQueue.isEmpty());

    const QString answer = QLatin1String("Bigos");
    alice.initSMPSecret(answer);
    QVERIFY(!alice.mesQueue.isEmpty());

    QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
    alice.mesQueue.clear();
    QVERIFY(bobEnv->smpQuery);
    QVERIFY(bob.mesQueue.empty());

    QVERIFY(bobEnv->question.isEmpty());

    bob.respondSMPAuthentication(QLatin1String("Pierogi"));
    QVERIFY(!bob.mesQueue.isEmpty());

    QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
    bob.mesQueue.clear();
    QVERIFY(!alice.mesQueue.empty());
    QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
    alice.mesQueue.clear();

    QVERIFY(!bob.mesQueue.isEmpty());
    QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
    QVERIFY(aliceEnv->smpFinished);
    QVERIFY(bobEnv->smpFinished);
    QVERIFY(!aliceEnv->smpSuccess);
    QVERIFY(!bobEnv->smpSuccess);
    QVERIFY(alice.mesQueue.empty());

    QCOMPARE(alice.trustLevel(), TrustLevel::UNVERIFIED);
    QCOMPARE(bob.trustLevel(), TrustLevel::UNVERIFIED);
    QVERIFY(aliceEnv->levelChanged);
    QVERIFY(bobEnv->levelChanged);
}

void OTRTest::testSMPQueryAborted()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    startSession(alice, bob);

    QVERIFY(alice.mesQueue.isEmpty());
    QVERIFY(bob.mesQueue.isEmpty());

    const QString question = QLatin1String("Best polish grindcore band?");
    const QString answer = QLatin1String("Antigama");
    alice.initSMPQuery(question, answer);
    QVERIFY(!alice.mesQueue.isEmpty());

    QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
    alice.mesQueue.clear();
    QVERIFY(bobEnv->smpQuery);
    QVERIFY(bob.mesQueue.empty());

    QCOMPARE(bobEnv->question, question);

    bob.abortSMPAuthentiaction();
    QVERIFY(!bob.mesQueue.empty());
    QCOMPARE(alice.decrypt(bob.mesQueue.back()), CryptResult::OTR);
    QVERIFY(aliceEnv->smpAborted);
    QVERIFY(!alice.mesQueue.empty());
    QCOMPARE(bob.decrypt(alice.mesQueue.back()), CryptResult::OTR);
    QVERIFY(bobEnv->smpAborted);
}

void OTRTest::testFingerprintManagement()
{

    tst::SessionEnv johnEnv(tst::johnCtx, bobMan);
    tst::SessionEnv aliceJEnv(tst::aliceJCtx, aliceMan);

    TestSession &alice = aliceEnv->ses;
    TestSession &aliceJ = aliceJEnv.ses;
    TestSession &bob = bobEnv->ses;
    TestSession &john = johnEnv.ses;

    startSession(alice, bob);
    startSession(aliceJ, john);
    qDebug() << (int)alice.trustLevel();
    qDebug() << (int)bob.trustLevel();

    KTp::FingerprintInfoList infoList = aliceMan->getKnownFingerprints(tst::aliceCtx.accountId);
    QCOMPARE(infoList.size(), 2);
    for(const auto &fpInfo: infoList) {
        QVERIFY(!fpInfo.isVerified);
        QVERIFY(fpInfo.inUse);
        if(fpInfo.contactName == alice.context().recipientName) {
            QCOMPARE(fpInfo.fingerprint, alice.remoteFingerprint());
        }
        if(fpInfo.contactName == aliceJ.context().recipientName) {
            QCOMPARE(fpInfo.fingerprint, aliceJ.remoteFingerprint());
        }
    }

    QVERIFY(aliceMan->trustFingerprint(alice.context().accountId,
                alice.context().recipientName, alice.remoteFingerprint(), true));
    QVERIFY(aliceMan->trustFingerprint(aliceJ.context().accountId,
                aliceJ.context().recipientName, aliceJ.remoteFingerprint(), true));

    infoList = aliceMan->getKnownFingerprints(tst::aliceCtx.accountId);
    QCOMPARE(infoList.size(), 2);
    for(const auto &fpInfo: infoList) {
        QVERIFY(fpInfo.isVerified);
        QVERIFY(fpInfo.inUse);
    }

    QVERIFY(aliceMan->trustFingerprint(alice.context().accountId,
               alice.context().recipientName, alice.remoteFingerprint(), false));
    QVERIFY(aliceMan->trustFingerprint(aliceJ.context().accountId,
               aliceJ.context().recipientName, aliceJ.remoteFingerprint(), false));

    infoList = aliceMan->getKnownFingerprints(tst::aliceCtx.accountId);
    QCOMPARE(infoList.size(), 2);
    for(const auto &fpInfo: infoList) {
        QVERIFY(!fpInfo.isVerified);
        QVERIFY(fpInfo.inUse);
    }

    stopSession(alice, bob);
    stopSession(aliceJ, john);

    infoList = aliceMan->getKnownFingerprints(tst::aliceCtx.accountId);
    QCOMPARE(infoList.size(), 2);
    for(const auto &fpInfo: infoList) {
        QVERIFY(!fpInfo.inUse);
    }

    startSession(alice, bob);
    startSession(aliceJ, john);
    // forget fingerprints

    // fingerprint is in use
    QVERIFY(!aliceMan->forgetFingerprint(alice.context().accountId,alice.context().recipientName, alice.remoteFingerprint()));
    QVERIFY(!aliceMan->forgetFingerprint(aliceJ.context().accountId, aliceJ.context().recipientName, aliceJ.remoteFingerprint()));

    const QString bobFp = alice.remoteFingerprint();
    const QString johnFp = aliceJ.remoteFingerprint();

    stopSession(alice, bob);
    stopSession(aliceJ, john);

    QVERIFY(aliceMan->forgetFingerprint(alice.context().accountId,alice.context().recipientName, bobFp));
    QVERIFY(aliceMan->forgetFingerprint(aliceJ.context().accountId, aliceJ.context().recipientName, johnFp));

    // nothing to forget
    QVERIFY(!aliceMan->forgetFingerprint(alice.context().accountId,alice.context().recipientName, bobFp));
    QVERIFY(!aliceMan->forgetFingerprint(aliceJ.context().accountId, aliceJ.context().recipientName, johnFp));

    infoList = aliceMan->getKnownFingerprints(tst::aliceCtx.accountId);
    QVERIFY(infoList.isEmpty());
}

void OTRTest::testDiffrentContentTypes()
{
    TestSession &alice = aliceEnv->ses;
    TestSession &bob = bobEnv->ses;

    startSession(alice, bob);

    Message helloMsg;
    const QString helloText = QLatin1String("No witej chopie!");
    const QString contentType = QLatin1String("text/html");
    helloMsg.setText(helloText, contentType);
    QCOMPARE(bob.encrypt(helloMsg), CryptResult::CHANGED);
    QVERIFY(bob.eventQueue.empty());

    QCOMPARE(alice.decrypt(helloMsg), CryptResult::CHANGED);
    QVERIFY(alice.eventQueue.empty());
    QCOMPARE(helloMsg.text(), helloText);
    QCOMPARE(helloMsg.contentType(), contentType);
}

void OTRTest::cleanup()
{
    delete aliceEnv;
    delete bobEnv;
    delete bobMan;
    delete aliceMan;
    QFile(aliceConfig.saveLocation() + tst::aliceCtx.accountId + QLatin1String(".fingerprints")).remove();
    QFile(bobConfig.saveLocation() + tst::bobCtx.accountId + QLatin1String(".fingerprints")).remove();
    QFile(bobConfig.saveLocation() + tst::johnCtx.accountId + QLatin1String(".fingerprints")).remove();
    aliceConfig.setPolicy(OTRL_POLICY_MANUAL);
    bobConfig.setPolicy(OTRL_POLICY_MANUAL);
}

QTEST_MAIN(OTRTest)
#include "otr-test.moc"
