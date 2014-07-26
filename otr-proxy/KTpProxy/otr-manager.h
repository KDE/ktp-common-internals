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

    class Manager
    {
        public:
            Manager(Config *otrConfig);

            UserStateBox* getUserState(const SessionContext &ctx);

            OtrlPolicy getPolicy() const;
            void setPolicy(OtrlPolicy policy);

            void createNewPrivateKey(Session *session);
            void saveFingerprints(Session *session);
            void createInstag(Session *session);

        private:
            Config *config;
            // TODO - consider clearing states when not in use
            QMap<QString, UserStateBoxPtr> userStates;
    };

} /* namespace OTR */

#endif