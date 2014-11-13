/*
    Copyright (C) 2012 George Kiagiadakis <kiagiadakis.george@gmail.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CAPABILITIES_HACK_PRIVATE_H
#define CAPABILITIES_HACK_PRIVATE_H

#include <KTp/ktpcommoninternals_export.h>
#include <TelepathyQt/CapabilitiesBase>

/*
 * This is a hack to workaround a gabble bug.
 * https://bugs.freedesktop.org/show_bug.cgi?id=51978
 */

namespace CapabilitiesHackPrivate {

/* Equivalent to caps.audioCalls() */
KTPCOMMONINTERNALS_NO_EXPORT bool audioCalls(const Tp::CapabilitiesBase &caps, const QString &cmName);

/* Equivalent to caps.videoCalls() */
KTPCOMMONINTERNALS_NO_EXPORT bool videoCalls(const Tp::CapabilitiesBase &caps, const QString &cmName);

}

#endif
