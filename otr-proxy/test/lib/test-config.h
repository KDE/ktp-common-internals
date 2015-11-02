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

#ifndef KTP_PROXY_OTR_TEST_CONFIG_HEADER
#define KTP_PROXY_OTR_TEST_CONFIG_HEADER

#include <KTpProxy/otr-config.h>

namespace OTR
{

    class TestConfig : public Config
    {
        public:
            TestConfig();

            virtual QString saveLocation();
            void setSaveLocation(const QString &sloc);
            virtual OtrlPolicy getPolicy() const;
            virtual void setPolicy(OtrlPolicy policy);

        private:
            QString sloc;
            OtrlPolicy policy;
    };

} /* namespace OTR */

#endif
