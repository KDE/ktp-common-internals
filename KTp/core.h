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

#include <KTp/ktpcommoninternals_export.h>

namespace KTp
{
class GlobalContactManager;

    KTPCOMMONINTERNALS_EXPORT bool kpeopleEnabled();

    /**
     * Returns an accountFactory with the following features set:
     *  - FeatureCore
     *  - FeatureProfile
     *
     * FeatureConnection is not set, accounts will be created with a null connection at startup
     *
     */

    KTPCOMMONINTERNALS_EXPORT Tp::AccountFactoryConstPtr accountFactory();

    /**
     * Returns a connectionFactory with the following features set:
     *  - FeatureCore
     *  - FeatureSelfContact
     *
     * FeatureRoster is expensive and should be called manually in becomeReady(Feature::Connection::FeatureRoster)
     *
     */

    KTPCOMMONINTERNALS_EXPORT Tp::ConnectionFactoryConstPtr connectionFactory();

    /**
     * Returns a default channelFactory with no special features.
     * Applications are expected to create a custom channel factories for their clientRegistrars
     */
    KTPCOMMONINTERNALS_EXPORT Tp::ChannelFactoryConstPtr channelFactory();

    /**
     * Returns a contactFactory with the following features set:
     *  - FeatureSimplePresence
     *  - FeatureCapabilities
     *  - FeatureClientTypes
     *  - FeatureAvatarData
     */
    KTPCOMMONINTERNALS_EXPORT Tp::ContactFactoryConstPtr contactFactory();

    KTPCOMMONINTERNALS_EXPORT Tp::AccountManagerPtr accountManager();
    KTPCOMMONINTERNALS_EXPORT KTp::GlobalContactManager* contactManager();
}
