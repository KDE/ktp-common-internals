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

#ifndef KTP_PROXY_OTR_CONSTANTS_HEADER
#define KTP_PROXY_OTR_CONSTANTS_HEADER

namespace OTR
{
    enum class TrustLevel : unsigned int
    {
        NOT_PRIVATE = 0,
        UNVERIFIED  = 1,
        VERIFIED    = 2,
        FINISHED    = 3
    };

    enum class MessageDirection : unsigned int
    {
        TO_PEER,
        FROM_PEER,
        INTERNAL
    };

    enum class CryptResult : unsigned int
    {
        UNCHANGED, // message was returned untouched
        CHANGED,   // message was either encrypted or decrypted
        OTR,       // message is an OTR specific message
        ERROR      // error during encryption or decryption operation
    };

    enum class TrustFpResult : unsigned int
    {
        OK,
        INVALID_FINGERPRINT, // when given fingerprint was in invalid format
        NO_SUCH_FINGERPRINT  // when could not find such fingerprint
    };

    template <typename T> unsigned int toUInt(T &&t)
    {
        return static_cast<unsigned int>(t);
    }
}

#endif
