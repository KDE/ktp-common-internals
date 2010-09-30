/*
 * This file is part of telepathy-integration-daemon
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
 * Copyright (C) 2010 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
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

#include "nepomuktelepathyservice.h"

#include "telepathyaccountmonitor.h"
#include <Nepomuk/ResourceManager>
#include <TelepathyQt4/Types>
#include <KPluginFactory>

using namespace Nepomuk;

TelepathyService::TelepathyService(QObject* parent, const QVariantList &)
    : Nepomuk::Service(parent, true)
{
    // Initialise Telepathy.
    Tp::registerTypes();

    // Create an instance of the Telepathy Account Monitor.
    TelepathyAccountMonitor *monitor = new TelepathyAccountMonitor(this);

    setServiceInitialized(true);
}

TelepathyService::~TelepathyService()
{
}


NEPOMUK_EXPORT_SERVICE( TelepathyService, "nepomuktelepathyservice" );
