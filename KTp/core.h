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
class GlobalContactManager;

    KTP_EXPORT bool kpeopleEnabled();


    /**
     * Returns an accountFactory with the following features set:
     *  - FeatureCore
     *  - FeatureProfile
     *
     * FeatureConnection is not set, accounts will be created with a null connection at startup
     *
     * Additional features _must_ be added before an AccountManager is created
     */

    //TODO should these be Tp::AccountFactoryConstrPtr. It's clearer API with the warning above, but potential to be limiting
    KTP_EXPORT Tp::AccountFactoryPtr accountFactory();

    /**
     * Returns a connectionFactory with the following features set:
     *  - FeatureCore
     *  - FeatureSelfContact
     *
     * FeatureRoster is expensive and should be called manually in becomeReady(Feature::Connection::FeatureRoster)
     *
     */

    KTP_EXPORT Tp::ConnectionFactoryPtr connectionFactory();

    /**
     * Returns a default channelFactory with no special features.
     * Applications are expected to create a custom channel factories for their clientRegistrars
     */
    KTP_EXPORT Tp::ChannelFactoryPtr channelFactory();

    /**
     * Returns a contactFactory with the following features set:
     *  - FeatureSimplePresence
     *  - FeatureCapabilities
     *  - FeatureClientTypes
     *  - FeatureAvatarData
     */
    KTP_EXPORT Tp::ContactFactoryPtr contactFactory();

    KTP_EXPORT Tp::AccountManagerPtr accountManager();
    KTP_EXPORT KTp::GlobalContactManager* contactManager();
}
