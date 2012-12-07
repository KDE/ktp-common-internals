/*
* Copyright (C) 2012 David Edmundson <kde@davidedmundson.co.uk>
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

#include "contact-factory.h"
#include "contact.h"

Tp::ContactFactoryPtr KTp::ContactFactory::create(const Tp::Features &features) {
    return Tp::ContactFactoryPtr(new KTp::ContactFactory(features));
}

KTp::ContactFactory::ContactFactory(const Tp::Features &features)
    : Tp::ContactFactory(features)
{

}

Tp::ContactPtr KTp::ContactFactory::construct(Tp::ContactManager *manager, const Tp::ReferencedHandles &handle, const Tp::Features &features, const QVariantMap &attributes) const
{
    return Tp::ContactPtr(new KTp::Contact(manager, handle, features, attributes));
}
