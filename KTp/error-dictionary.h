/*
    Telepathy error dictionary - usable strings for dbus messages
    Copyright (C) 2011  Martin Klapetek <martin.klapetek@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef ERROR_DICTIONARY_H
#define ERROR_DICTIONARY_H

#include <QString>

#include <KTp/ktp-export.h>

namespace KTp
{
/**
 * @short A method of turning error codes into human readable strings
 *
 */
namespace ErrorDictionary
{
    /**Returns a verbose error message suitable for displaying to the user
        @param dbusErrorName The Telepathy error as specified in http://telepathy.freedesktop.org/spec/errors.html
     */
    KTP_EXPORT QString displayVerboseErrorMessage(const QString &dbusErrorName);

    /**Returns a short error message suitable when there is little space
        @param dbusErrorName The Telepathy error as specified in http://telepathy.freedesktop.org/spec/errors.html
     */
    KTP_EXPORT QString displayShortErrorMessage(const QString &dbusErrorName);
}
}

#endif // ERROR_DICTIONARY_H
