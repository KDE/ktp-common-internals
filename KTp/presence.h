/*
 * Global Presence - wrap Tp::Presence with KDE functionality
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2012 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
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

#ifndef PRESENCE_H
#define PRESENCE_H

#include <TelepathyQt/Presence>

#include <KIcon>

#include <KTp/ktp-export.h>

namespace KTp
{

class KTP_EXPORT Presence : public Tp::Presence
{
public:
    Presence();
    Presence(const Tp::Presence &presence);
    KIcon icon(bool useImIcons = false) const;
    KIcon icon(QStringList overlays, bool useImIcons = false) const;
    QString iconString(bool useImIcons = false) const;

    /** Returns which presence is "more available" */
    bool operator <(const Presence &other) const;

    QString displayString() const;

    /** Returns an int representation of the presence type sorted by priority. 0 - most online, 7 - offline */
    static int sortPriority(const Tp::ConnectionPresenceType &type);
};

}

Q_DECLARE_METATYPE(KTp::Presence);

#endif // PRESENCE_H
