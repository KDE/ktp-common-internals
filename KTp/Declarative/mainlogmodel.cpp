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

#include "mainlogmodel.h"

#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QStandardPaths>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingChannel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Channel>
#include <TelepathyQt/ChannelClassSpecList>

#include <KPeople/PersonData>

#include "conversation.h"

#include <QDebug>

static inline Tp::ChannelClassSpecList channelClassList()
{
    return Tp::ChannelClassSpecList() << Tp::ChannelClassSpec::textChat();
}

ObserverProxy::ObserverProxy(MainLogModel *model)
    : QObject(model),
      Tp::AbstractClientObserver(channelClassList(), true),
      m_model(model)
{

}

void ObserverProxy::observeChannels(const Tp::MethodInvocationContextPtr<> &context,
                                    const Tp::AccountPtr &account,
                                    const Tp::ConnectionPtr &connection,
                                    const QList<Tp::ChannelPtr> &channels,
                                    const Tp::ChannelDispatchOperationPtr &dispatchOperation,
                                    const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                                    const Tp::AbstractClientObserver::ObserverInfo &observerInfo)
{
    Q_UNUSED(context)
    Q_UNUSED(connection)
    Q_UNUSED(requestsSatisfied)
    Q_UNUSED(observerInfo)

    Q_FOREACH(const Tp::ChannelPtr &channel, channels) {
        Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (textChannel) {
            textChannel.data()->setProperty("dispatchOperation", QVariant::fromValue(dispatchOperation));
            m_model->handleChannel(account, textChannel);
        }
    }
}

// -----------------------------------------------------------------------

MainLogModel::MainLogModel(QObject *parent)
    : QAbstractListModel(parent),
      Tp::AbstractClientHandler(channelClassList()),
      m_observerProxy(new ObserverProxy(this))
{
    QCommandLineParser parser;
    parser.process(qApp->arguments());

    m_openIncomingChannel = parser.isSet(QStringLiteral("openIncomingChannel"));

    const QString dbLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/ktp-mobile-logger/");

    connect(qApp, &QCoreApplication::aboutToQuit, this, [=]() {
        Q_FOREACH (Conversation *c, m_conversations.values()) {
            if (!c->textChannel().isNull()) {
                c->textChannel()->requestClose();
            }
        }
    });

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("logger-db"));
    m_db.setDatabaseName(dbLocation + QStringLiteral("history.db3"));
    qDebug() << dbLocation << m_db.open();
    m_query = QSqlQuery(QStringLiteral("SELECT data.messageDateTime, data.message, "
                                       "accountData.accountObjectPath, contactData.targetContact "
                                       "FROM data LEFT JOIN contactData ON data.targetContactId = contactData.id "
                                       "LEFT JOIN accountData ON data.accountId = accountData.id "
                                       "GROUP BY data.targetContactId ORDER BY data.messageDateTime DESC"),
                        m_db);

    m_query.exec();

    // The query results are processed as soon as AccountManager
    // is passsed to MainLogModel
}

MainLogModel::~MainLogModel()
{

}

void MainLogModel::processQueryResults(QSqlQuery query)
{
    while (query.next()) {
        LogItem item;
        item.messageDateTime = query.value(QStringLiteral("messageDateTime")).toDateTime();
        item.message = query.value(QStringLiteral("message")).toString();
        item.accountObjectPath = query.value(QStringLiteral("accountObjectPath")).toString();
        item.targetContact = query.value(QStringLiteral("targetContact")).toString();

        const QString accountObjectPath = item.accountObjectPath.mid(35);
        item.conversation = new Conversation(item.targetContact, m_accountManager->accountForObjectPath(item.accountObjectPath), this);

        m_conversations.insert(accountObjectPath + item.targetContact, item.conversation);
        setupSignals(item.conversation);

        //TODO: This might be more effective to insert at once?
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_logItems << item;
        endInsertRows();
    }
}

QVariant MainLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const int row = index.row();

    switch (role) {
        case MainLogModel::ContactIdRole:
            return m_logItems.at(row).targetContact;
        case MainLogModel::AccountIdRole:
            return m_logItems.at(row).accountObjectPath.mid(35);
        case MainLogModel::LastMessageDateRole:
        case MainLogModel::LastMessageTextRole:
        case MainLogModel::ConversationRole:
        case MainLogModel::HasUnreadMessagesRole:
        case MainLogModel::UnreadMessagesCountRole:
        case MainLogModel::ContactDisplayNameRole:
        case MainLogModel::PersonUriRole:
        {
            if (role == MainLogModel::ConversationRole) {
                return QVariant::fromValue(m_logItems.at(row).conversation);
            }

            const Conversation *conversation = m_logItems.at(row).conversation;

            if (!conversation->personData()->isValid()) {
                if (role == MainLogModel::PersonUriRole || role == MainLogModel::ContactDisplayNameRole) {
                    return QVariant();
                }
            } else {
                if (role == MainLogModel::PersonUriRole) {
                    return conversation->personData()->personUri();
                } else if (role == MainLogModel::ContactDisplayNameRole) {
                    return conversation->personData()->name();
                }
            }

            if (!conversation->isValid()) {
                if (role == MainLogModel::HasUnreadMessagesRole) {
                    return false;
                } else if (role == MainLogModel::UnreadMessagesCountRole) {
                    // TODO this needs to be replaced once the persistent
                    //      unread count is done
                    return 0;
                } else if (role == MainLogModel::LastMessageDateRole) {
                    return m_logItems.at(row).messageDateTime;
                } else if (role == MainLogModel::LastMessageTextRole) {
                    return m_logItems.at(row).message;
                }
            } else {
                if (role == MainLogModel::HasUnreadMessagesRole) {
                    return conversation->hasUnreadMessages();
                } else if (role == MainLogModel::UnreadMessagesCountRole) {
                    return conversation->messages()->unreadCount();
                } else if (role == MainLogModel::LastMessageDateRole) {
                    return conversation->messages()->lastMessageDateTime();
                } else if (role == MainLogModel::LastMessageTextRole) {
                    return conversation->messages()->lastMessage();
                }
            }
        }
    }

    return QVariant();
}

QVariant MainLogModel::data(int index, QByteArray role) const
{
    return data(createIndex(index, 0), roleNames().key(role));
}

int MainLogModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_logItems.size();
}

QHash<int, QByteArray> MainLogModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    roles.insert(ContactDisplayNameRole, "contactDisplayName");
    roles.insert(ContactIdRole, "contactId");
    roles.insert(PersonUriRole, "personUri");
    roles.insert(AccountIdRole, "accountId");
    roles.insert(LastMessageDateRole, "lastMessageDate");
    roles.insert(LastMessageTextRole, "lastMessageText");
    roles.insert(ConversationRole, "conversation");
    roles.insert(HasUnreadMessagesRole, "hasUnreadMessages");
    roles.insert(UnreadMessagesCountRole, "unreadMessagesCount");

    return roles;
}

bool MainLogModel::canChat(const QString &accountId) const
{
    if (m_accountManager.isNull()) {
        return false;
    }

    const QString objectPath = TP_QT_ACCOUNT_OBJECT_PATH_BASE + QLatin1Char('/') + accountId;
    const Tp::AccountPtr account = m_accountManager->accountForObjectPath(objectPath);

    if (!account.isNull() && account->currentPresence().type() != Tp::ConnectionPresenceTypeOffline) {
        return true;
    }

    return false;
}

void MainLogModel::startChat(const QString &accountId, const QString &contactId)
{
    const QString objectPath = TP_QT_ACCOUNT_OBJECT_PATH_BASE + QLatin1Char('/') + accountId;
    const Tp::AccountPtr account = m_accountManager->accountForObjectPath(objectPath);

    if (account.isNull()) {
        qWarning() << "Cannot get account for" << accountId;
    }

    if (m_conversations.contains(accountId + contactId)) {
        Conversation *conversation = m_conversations.value(accountId + contactId);
        if (conversation->isValid() && !conversation->textChannel().isNull()) {
            Tp::ChannelDispatchOperationPtr dispatch = conversation->textChannel().data()->property("dispatchOperation").value<Tp::ChannelDispatchOperationPtr>();

            if (!dispatch.isNull()) {
                // Claim the channel that is being observed
                dispatch->claim();
            }
            // We already have a conversation, don't request new channel
            return;
        }
    }

    Tp::PendingChannel *pendingChannel = account->ensureAndHandleTextChat(contactId);
    connect(pendingChannel, &Tp::PendingChannel::finished, [=](Tp::PendingOperation *op) {
        if (op->isError()) {
            qWarning() << "Requesting text channel failed:" << op->errorName() << op->errorMessage();
            return;
        }

        Tp::PendingChannel *pc = qobject_cast<Tp::PendingChannel*>(op);
        if (pc) {
            Tp::TextChannel *channel = qobject_cast<Tp::TextChannel*>(pc->channel().data());
            handleChannel(account, Tp::TextChannelPtr(channel));
        }
    });
}

void MainLogModel::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    m_accountManager = accountManager;

    processQueryResults(m_query);
}

QObject* MainLogModel::observerProxy() const
{
    return m_observerProxy;
}

void MainLogModel::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                                        const Tp::AccountPtr &account,
                                        const Tp::ConnectionPtr &connection,
                                        const QList<Tp::ChannelPtr> &channels,
                                        const QList<Tp::ChannelRequestPtr> &channelRequests,
                                        const QDateTime &userActionTime,
                                        const HandlerInfo &handlerInfo)
{
    Q_UNUSED(connection);
    Q_UNUSED(channelRequests);
    Q_UNUSED(userActionTime);
    Q_UNUSED(handlerInfo);

    //check that the channel is of type text
    Tp::TextChannelPtr textChannel;
    Q_FOREACH (const Tp::ChannelPtr &channel, channels) {
        textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (textChannel) {
            break;
        }
    }

    Q_ASSERT(textChannel);

    const QString targetContact = textChannel->targetContact()->id();
    const QString accountObjectPath = account->objectPath();

    bool existsInModel = false;
    Q_FOREACH (const LogItem &item, m_logItems) {
        if (item.targetContact == targetContact && item.accountObjectPath == accountObjectPath) {
            existsInModel = true;
            break;
        }
    }

    if (!existsInModel) {
        QSqlQuery q(m_db);
        q.prepare(QStringLiteral("SELECT data.messageDateTime, data.message, "
                                 "accountData.accountObjectPath, contactData.targetContact "
                                 "FROM data LEFT JOIN contactData ON data.targetContactId = contactData.id "
                                 "LEFT JOIN accountData ON data.accountId = accountData.id "
                                 "WHERE contactData.targetContact = :contactId AND accountData.accountObjectPath = :accountObjectPath "
                                 "GROUP BY data.targetContactId ORDER BY data.messageDateTime DESC"));
        q.bindValue(QStringLiteral(":contactId"), targetContact);
        q.bindValue(QStringLiteral(":accountObjectPath"), accountObjectPath);

        q.exec();

        if (q.lastError().isValid()) {
            qWarning() << "Error selecting latest conversation from log database:" << q.lastError().text();
        }

        processQueryResults(q);
    }

    handleChannel(account, textChannel);
    context->setFinished();
}

bool MainLogModel::bypassApproval() const
{
    return true;
}

void MainLogModel::handleChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel)
{
    if (channel && account) {
        const QString accountId = account->objectPath().mid(35);
        const QString contactId = channel->targetContact()->id();
        qDebug() << accountId << contactId;

        int i = 0;
        for (i = 0; i < m_logItems.size(); i++) {
            LogItem &item = m_logItems[i];
            if (item.targetContact == contactId && item.accountObjectPath == account->objectPath()) {
                // Don't set the same channel again
                if (item.conversation->textChannel() == channel) {
                    return;
                }

                item.conversation->setTextChannel(channel);
                break;
            }
        }

        QModelIndex contactIndex = createIndex(i, 0);

        // No existing log item was found and it's a local request,
        // do a model insert
        if (i == m_logItems.size()) {
            LogItem item;
            item.targetContact = contactId;
            item.accountObjectPath = account->objectPath();

            Conversation *conversation = new Conversation(contactId, account, this);
            item.conversation = conversation;
            setupSignals(conversation);
            m_conversations.insert(accountId + contactId, conversation);

            conversation->setTextChannel(channel);

            beginInsertRows(QModelIndex(), m_logItems.size(), m_logItems.size());
            m_logItems << item;
            endInsertRows();
        } else {
            if (contactIndex.isValid()) {
                Q_EMIT dataChanged(contactIndex, contactIndex);
            }
        }

        if (channel->isRequested() || m_openIncomingChannel) {
            Q_EMIT newRequestedChannel(contactIndex);
            m_openIncomingChannel = false;
        }
    }
}

void MainLogModel::setupSignals(Conversation *conversation) const
{
    connect(conversation, &Conversation::unreadMessagesChanged, this, &MainLogModel::onConversationChanged);
    connect(conversation, &Conversation::avatarChanged, this, &MainLogModel::onConversationChanged);
    connect(conversation, &Conversation::presenceIconChanged, this, &MainLogModel::onConversationChanged);
    connect(conversation, &Conversation::titleChanged, this, &MainLogModel::onConversationChanged);
    connect(conversation, &Conversation::validityChanged, this, &MainLogModel::onConversationChanged);
    connect(conversation, &Conversation::lastMessageChanged, this, &MainLogModel::onConversationChanged);
}

void MainLogModel::onConversationChanged()
{
    Conversation *conversation = qobject_cast<Conversation*>(sender());
    if (!conversation || !conversation->isValid()) {
        return;
    }

    int i = 0;
    for (i = 0; i < m_logItems.size(); i++) {
        if (m_logItems.at(i).conversation == conversation) {
            break;
        }
    }

    const QModelIndex index = createIndex(i, 0);

    if (index.isValid()) {
        Q_EMIT dataChanged(index, index);
    }
}
