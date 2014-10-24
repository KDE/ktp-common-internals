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

#include "test-config.h"

namespace OTR
{

    TestConfig::TestConfig()
        : sloc(QLatin1String("./resources/")),
        policy(OTRL_POLICY_MANUAL)
    {
    }

    QString TestConfig::saveLocation()
    {
        return sloc;
    }

    void TestConfig::setSaveLocation(const QString &sloc)
    {
        this->sloc = sloc;
    }

    OtrlPolicy TestConfig::getPolicy() const
    {
        return policy;
    }

    void TestConfig::setPolicy(OtrlPolicy policy)
    {
        this->policy = policy;
    }

} /* namespace OTR */
