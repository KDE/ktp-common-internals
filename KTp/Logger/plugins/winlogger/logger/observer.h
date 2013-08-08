/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef HANDLER_H
#define HANDLER_H

#include <TelepathyQt/AbstractClientObserver>

class Observer : public QObject, public Tp::AbstractClientObserver
{
    Q_OBJECT
  public:
    Observer(QObject *parent = 0);
    ~Observer();

    void observeChannels(const Tp::MethodInvocationContextPtr<> &context,
                         const Tp::AccountPtr &account,
                         const Tp::ConnectionPtr &connection,
                         const QList<Tp::ChannelPtr> &channels,
                         const Tp::ChannelDispatchOperationPtr &dispatchOperation,
                         const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                         const ObserverInfo &observerInfo);
};

#endif // HANDLER_H
