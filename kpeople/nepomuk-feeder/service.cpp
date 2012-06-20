/*
 * This file is part of telepathy-nepomuk-service
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

#include "service.h"

#include "controller.h"
#include "nepomuk-storage.h"

#include <KDebug>
#include <KPluginFactory>

#include <Nepomuk2/ResourceManager>

#include <TelepathyQt/Types>

TelepathyService::TelepathyService(QObject *parent, const QVariantList &)
    : Nepomuk2::Service(parent, true)
{
    // Initialise Telepathy.
    Tp::registerTypes();

    // Create an instance of the Telepathy Account Monitor.
    m_controller = new Controller(new NepomukStorage(), this);

    connect(m_controller,
            SIGNAL(storageInitialisationFailed()),
            SLOT(onStorageInitialisationFailed()));

    setServiceInitialized(true);

    kDebug() << "We're off...";
}

TelepathyService::~TelepathyService()
{
    m_controller->shutdown();
}

void TelepathyService::onStorageInitialisationFailed()
{
    kDebug() << "Storage initialisation failed. Terminate the service.";

    // Terminate the service
    deleteLater();
}


NEPOMUK_EXPORT_SERVICE( TelepathyService, "nepomuktelepathyservice" );


#include "service.moc"

