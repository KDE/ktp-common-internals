/*
    Copyright (C) 2016  Martin Klapetek <mklapetek@kde.org>

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

#ifndef MAINLOGMODEL_H
#define MAINLOGMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QSqlQuery>

#include <TelepathyQt/AbstractClientObserver>
#include <TelepathyQt/AbstractClientHandler>
#include <TelepathyQt/ChannelDispatchOperation>

#include <KTp/persistent-contact.h>
#include <KTp/types.h>

class Conversation;
class MainLogModel; // Cause of ObserverProxy

class LogItem {
public:
    QDateTime messageDateTime;
    QString message;
    QString accountObjectPath;
    QString targetContact;
    Conversation *conversation;
};

/**
 * The reason for this class is that an Observer and a Handler cannot
 * be registered under the same client name if the Observer is not to
 * be autostarted and only monitor things once the app is executed.
 *
 * So this is a tiny proxy class that gets registered as SpaceBarObserverProxy
 * and forwards all observerChannels calls to the model which then merges
 * them with the existing conversations
 */
class ObserverProxy : public QObject, public Tp::AbstractClientObserver
{
    Q_OBJECT

public:
    ObserverProxy(MainLogModel *model);

    void observeChannels(const Tp::MethodInvocationContextPtr<> &context,
                         const Tp::AccountPtr &account,
                         const Tp::ConnectionPtr &connection,
                         const QList<Tp::ChannelPtr> &channels,
                         const Tp::ChannelDispatchOperationPtr &dispatchOperation,
                         const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                         const Tp::AbstractClientObserver::ObserverInfo &observerInfo);

private:
    MainLogModel *m_model;
};

//-----------------------------------------------------------------------------

class MainLogModel : public QAbstractListModel, public Tp::AbstractClientHandler
{
    Q_OBJECT

public:
    enum Role {
        ContactDisplayNameRole = Qt::DisplayRole,
        ContactIdRole = Qt::UserRole,
        PersonUriRole,
        AccountIdRole,
        LastMessageDateRole,
        LastMessageTextRole,
        ConversationRole,
        HasUnreadMessagesRole,
        UnreadMessagesCountRole,

        UserRole = Qt::UserRole + 0x1000 ///< in case it's needed to extend, use this one to start from
    };
    Q_ENUMS(Role)

    MainLogModel(QObject *parent = 0);
    ~MainLogModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    Q_INVOKABLE bool canChat(const QString &accountId) const;
    Q_INVOKABLE void startChat(const QString &accountId, const QString &contactId);
    Q_INVOKABLE void setAccountManager(const Tp::AccountManagerPtr &accountManager);
    Q_INVOKABLE QVariant data(int index, QByteArray role) const;
    Q_INVOKABLE QObject* observerProxy() const;

    void handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                        const Tp::AccountPtr &account,
                        const Tp::ConnectionPtr &connection,
                        const QList<Tp::ChannelPtr> &channels,
                        const QList<Tp::ChannelRequestPtr> &channelRequests,
                        const QDateTime &userActionTime,
                        const HandlerInfo &handlerInfo);

    bool bypassApproval() const;

Q_SIGNALS:
    void newRequestedChannel(const QModelIndex &index);

private Q_SLOTS:
    void handleChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel);
    void onConversationChanged();

private:
    void setupSignals(Conversation *conversation) const;
    void processQueryResults(QSqlQuery query);

    QHash<QString, Conversation*> m_conversations; // This is a hash with keys "accountId + contactId"
    QList<LogItem> m_logItems;
    QSqlQuery m_query;
    QSqlDatabase m_db;
    Tp::AccountManagerPtr m_accountManager;
    ObserverProxy *m_observerProxy;

    friend class ObserverProxy;
};

Q_DECLARE_METATYPE(Tp::ChannelDispatchOperationPtr)

#endif
