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

#ifndef KTP_TEXT_CHANNEL_WATCHER_PROXY_MODEL_H
#define KTP_TEXT_CHANNEL_WATCHER_PROXY_MODEL_H

#include <TelepathyQt/AbstractClientObserver>
#include <TelepathyQt/TextChannel>

#include <QIdentityProxyModel>

#include <ktpmodels_export.h>

namespace KTp
{

/** This class proxies the ContactsListModel, adding a role for ongoing chats*/
class KTPMODELS_EXPORT TextChannelWatcherProxyModel : public QIdentityProxyModel, public Tp::AbstractClientObserver
{
    Q_OBJECT
public:
    explicit TextChannelWatcherProxyModel(QObject *parent=0);
    virtual ~TextChannelWatcherProxyModel();

    virtual void observeChannels(const Tp::MethodInvocationContextPtr<> &context,
                                 const Tp::AccountPtr &account,
                                 const Tp::ConnectionPtr &connection,
                                 const QList<Tp::ChannelPtr> &channels,
                                 const Tp::ChannelDispatchOperationPtr &dispatchOperation,
                                 const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                                 const Tp::AbstractClientObserver::ObserverInfo &observerInfo);

    QVariant data(const QModelIndex &proxyIndex, int role) const;

private Q_SLOTS:
    void onChannelMessagesChanged();
    void onChannelInvalidated();

private:
    class Private;
    Private *d;
};

}

#endif // KTP_TEXT_CHANNEL_WATCHER_PROXY_MODEL_H
