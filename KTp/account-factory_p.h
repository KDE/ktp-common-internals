/*
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

#ifndef KTP_ACCOUNT_FACTORY_H
#define KTP_ACCOUNT_FACTORY_H

#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/Account>

#include <TelepathyQt/Types>

#include <KTp/ktpcommoninternals_export.h>

/*
 * This class is part of a hack that fixes the following problem:
 *  Connection has no way to access the account that owns it
 *  This means contacts have no way to know where they are from
 *  For KPeople (and in other cases) we need the account UID from a contact
 *
 *
 * This class subclasses account and sets a property Connection whenever it is
 * created.
 */

namespace KTp {
class AccountFactory : public Tp::AccountFactory
{
public:
    static Tp::AccountFactoryPtr create(const QDBusConnection &bus, const Tp::Features &features=Tp::Features());
protected:
    AccountFactory(const QDBusConnection& bus, const Tp::Features& features);
    virtual Tp::AccountPtr construct(const QString& busName,
                                     const QString& objectPath,
                                     const Tp::ConnectionFactoryConstPtr& connFactory,
                                     const Tp::ChannelFactoryConstPtr& chanFactory,
                                     const Tp::ContactFactoryConstPtr& contactFactory) const;
};

class Account: public Tp::Account
{
    Q_OBJECT
public:
    static Tp::AccountPtr create (const QDBusConnection &bus, const QString &busName, const QString &objectPath, const Tp::ConnectionFactoryConstPtr &connectionFactory, const Tp::ChannelFactoryConstPtr &channelFactory, const Tp::ContactFactoryConstPtr &contactFactory=Tp::ContactFactory::create());
protected:
    Account (const QDBusConnection &bus,
             const QString &busName,
             const QString &objectPath,
             const Tp::ConnectionFactoryConstPtr &connectionFactory,
             const Tp::ChannelFactoryConstPtr &channelFactory,
             const Tp::ContactFactoryConstPtr &contactFactory,
             const Tp::Feature &coreFeature);

private Q_SLOTS:
    void onConnectionChanged(const Tp::ConnectionPtr &connection);
};
}


#endif // CONTACTFACTORY_H
