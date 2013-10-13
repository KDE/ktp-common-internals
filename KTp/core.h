/*
 * static methods on the KTp namespace
 *
 * Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
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

#include <TelepathyQt/Types>

#include <KTp/ktp-export.h>

namespace KTp
{
    KTP_EXPORT bool kpeopleEnabled();


    //FIXME for 0.8
    /*  For 0.8 we have a plan to have a singleton accountManager
        (or accountFactory/contactFactory/channelFactory/connectionFactory).

        We didn't get this done for everyone to use this in time for 0.7.
        However we still need the contact list and kpeople to share the connectionFactory
        This is an initial half-completed version.

        It is best not to use this method. It's only half complete.
     */
    KTP_EXPORT Tp::AccountManagerPtr accountManager();
}
