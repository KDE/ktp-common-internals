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

#ifndef KTP_PROXY_OTR_MANAGER_HEADER
#define KTP_PROXY_OTR_MANAGER_HEADER

#include "otr-config.h"
#include "otr-session.h"

#include "KTp/OTR/types.h"

#include <QThread>

extern "C" {
#include <gcrypt.h>
#include <libotr/privkey.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
}

namespace OTR
{
    namespace global
    {
        extern const OtrlMessageAppOps appOps;
    }

    class KeyGenerationWorker;

    class Manager : public QObject
    {
        Q_OBJECT

        public:
            Manager(Config *otrConfig);

            UserStateBox* getUserState(const QString &accountId);

            OtrlPolicy getPolicy() const;
            void setPolicy(OtrlPolicy policy);

            void saveFingerprints(const QString &accountId);
            void saveFingerprints(Session *session);
            TrustFpResult trustFingerprint(const SessionContext &ctx, Fingerprint *fingerprint, bool trust);

            void createNewPrivateKey(Session *session);
            /** return nullptr if thread could not be craeted
              otherwise returns thread which generates a new private key upon calling start method */
            KeyGenerationWorker* createNewPrivateKey(const QString &accountId, const QString &accountName);
            QString getFingerprintFor(const QString &accountId, const QString &accountName);
            KTp::FingerprintInfoList getKnownFingerprints(const QString &accountId);
            bool trustFingerprint(const QString &accountId, const QString &contactName, const QString &fingerprint, bool trust);
            bool forgetFingerprint(const QString &accountId, const QString &contactName, const QString &fingerprint);

            void createInstag(Session *session);

        Q_SIGNALS:
            void fingerprintTrusted(const QString &accountId, const QString &fingerprint, bool trusted);

        private:
            Config *config;
            // TODO - consider clearing states when not in use
            QMap<QString, UserStateBoxPtr> userStates;
    };

    class KeyGenerationWorker : public QObject
    {
        Q_OBJECT

        public:
            KeyGenerationWorker(
                    const QString &accountId,
                    const QString &accountName,
                    const QString &protocol,
                    const QString &path,
                    OtrlUserState userState);

            gcry_error_t error() const;
            /* has to called before the thread starts*/
            gcry_error_t prepareCreation();
            /* has to called after the finished() signal is emitted */
            gcry_error_t finalizeCreation();

        public Q_SLOTS:
            void calculate();

        Q_SIGNALS:
            void finished();

        public:
            const QString accountId;
            const QString accountName;
            const QString protocol;
            const QString path;

        private:
            OtrlUserState userState;
            gcry_error_t err;
            void *newKey;
    };

} /* namespace OTR */

#endif
