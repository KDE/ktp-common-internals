/*
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

#ifndef KTP_DEBUG_H
#define KTP_DEBUG_H

#include "ktpcommoninternals_export.h"

namespace KTp {
namespace Debug {

    /**
     * Installs Telepathy-Qt debug callback and enable/disable Telepathy-Qt
     * debug and warning output
     *
     * @param debug If true enable Telepathy-Qt debug
     * @param warning If true enable Telepathy-Qt warnings
     */
    KTPCOMMONINTERNALS_EXPORT void installCallback(bool debug, bool warning = true);

} // namespace Debug
} // namespace KTp

#endif // KTP_DEBUG_H
