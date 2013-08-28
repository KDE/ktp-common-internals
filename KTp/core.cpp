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

#include "core.h"

#ifdef HAVE_KPEOPLE
#include <Nepomuk2/ResourceManager>
#endif

#include <KGlobal>

class CorePrivate
{
public:
    CorePrivate();
    bool m_kPeopleEnabled;
};

CorePrivate::CorePrivate()
    : m_kPeopleEnabled(false)
{
    //if built with kpeople support, enable kpeople if Nepomuk is running
    #ifdef HAVE_KPEOPLE
        m_kPeopleEnabled = Nepomuk2::ResourceManager::instance()->initialized();
    #endif
}

K_GLOBAL_STATIC(CorePrivate, s_instance)

bool KTp::kpeopleEnabled()
{
    return s_instance->m_kPeopleEnabled;
}
