/*
 * This file is part of telepathy-integration-daemon
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
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

#include "telepathycontact.h"

#include "telepathyaccount.h"

#include <KDebug>

TelepathyContact::TelepathyContact(Tp::ContactPtr contact,
                                   Tp::ConnectionPtr connection,
                                   TelepathyAccount *parent)
 : QObject(parent),
   m_parent(parent),
   m_contact(contact),
   m_connection(connection)
{
    kDebug() << "New TelepathyContact Created:"
             << m_contact.data()
             << m_connection.data()
             << m_parent;

    // We need to destroy ourself if the connection goes down.
    connect(m_connection.data(),
            SIGNAL(invalidated(Tp::DBusProxy*, const QString&, const QString&)),
            SLOT(deleteLater()));
}

TelepathyContact::~TelepathyContact()
{
    kDebug();
}


#include "telepathycontact.moc"

