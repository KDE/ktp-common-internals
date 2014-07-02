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

#include "presence.h"

#include <KLocalizedString>
#include <KIconLoader>

namespace KTp
{

Presence::Presence() :
    Tp::Presence()
{
}

Presence::Presence(const Tp::Presence &presence) :
    Tp::Presence(presence)
{
}

KIcon Presence::icon(bool useImIcons) const
{
    const QString &name(iconName(useImIcons));
    if (!name.isEmpty()) {
        return KIcon(name);
    }
    return KIcon();
}
KIcon Presence::icon(QStringList overlays, bool useImIcons) const
{
    const QString &name(iconName(useImIcons));
    if (!name.isEmpty()) {
        return KIcon(name,
                     KIconLoader::global(),
                     overlays);
    }
    return KIcon();
}

QString Presence::iconName(bool useImIcons) const
{
    switch (type()) {
    case Tp::ConnectionPresenceTypeAvailable:
        return useImIcons ? QLatin1String("im-user") : QLatin1String("user-online");
    case Tp::ConnectionPresenceTypeBusy:
        return useImIcons ? QLatin1String("im-user-busy") : QLatin1String("user-busy");
    case Tp::ConnectionPresenceTypeAway:
        return useImIcons ? QLatin1String("im-user-away") : QLatin1String("user-away");
    case Tp::ConnectionPresenceTypeExtendedAway:
        // FIXME Request an icon "im-user-away-extended"
        return useImIcons ? QLatin1String("im-user-away") : QLatin1String("user-away-extended");
    case Tp::ConnectionPresenceTypeHidden:
        return useImIcons ? QLatin1String("im-invisible-user") : QLatin1String("user-invisible");
    case Tp::ConnectionPresenceTypeOffline:
        return useImIcons ? QLatin1String("im-user-offline") : QLatin1String("user-offline");
    default:
        return useImIcons ? QLatin1String("im-user-offline") : QLatin1String("user-offline");
    }
}

bool Presence::operator <(const Presence &other) const
{
    if (sortPriority(type()) < sortPriority(other.type())) {
        return true;
    } else if (sortPriority(type()) == sortPriority(other.type())) {
        return (statusMessage() < other.statusMessage());
    } else {
        return false;
    }
}

QString Presence::displayString() const
{
    switch (type()) {
        case Tp::ConnectionPresenceTypeAvailable:
            return i18nc("IM presence: a person is available", "Available");
        case Tp::ConnectionPresenceTypeBusy:
            return i18nc("IM presence: a person is busy", "Busy");
        case Tp::ConnectionPresenceTypeAway:
            return i18nc("IM presence: a person is away", "Away");
        case Tp::ConnectionPresenceTypeExtendedAway:
            return i18nc("IM presence: a person is not available", "Not Available");
        case Tp::ConnectionPresenceTypeHidden:
            return i18nc("IM presence: a person is invisible", "Invisible");
        case Tp::ConnectionPresenceTypeOffline:
            return i18nc("IM presence: a person is offline", "Offline");
        default:
            return QString();
    }
}

int Presence::sortPriority(const Tp::ConnectionPresenceType &type)
{
    switch(type) {
        case Tp::ConnectionPresenceTypeAvailable:
            return 0;
        case Tp::ConnectionPresenceTypeBusy:
            return 1;
        case Tp::ConnectionPresenceTypeHidden:
            return 2;
        case Tp::ConnectionPresenceTypeAway:
            return 3;
        case Tp::ConnectionPresenceTypeExtendedAway:
            return 4;
        //don't distinguish between the following three presences
        case Tp::ConnectionPresenceTypeError:
        case Tp::ConnectionPresenceTypeUnknown:
        case Tp::ConnectionPresenceTypeUnset:
            return 5;
        case Tp::ConnectionPresenceTypeOffline:
        default:
            return 6;
    }

    return -1;
}

}
