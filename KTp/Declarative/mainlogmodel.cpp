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
#include <QSqlDatabase>
#include <QStandardPaths>
// #include <QDBusArgument>

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

MainLogModel::MainLogModel(QObject *parent)
    : QIdentityProxyModel(parent),
      Tp::AbstractClientHandler(channelClassList()),
      m_dbModel(new QSqlQueryModel(this))
{
    const QString dbLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/ktp-mobile-logger/");

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("logger-db"));
    m_db.setDatabaseName(dbLocation + QStringLiteral("history.db3"));
    qDebug() << dbLocation << m_db.open();
    m_query = QSqlQuery(QStringLiteral("SELECT data.messageDateTime, data.message, "
                                       "accountData.accountObjectPath, contactData.targetContact "
                                       "FROM data LEFT JOIN contactData ON data.targetContactId = contactData.id "
                                       "LEFT JOIN accountData ON data.accountId = accountData.id "
                                       "GROUP BY data.targetContactId ORDER BY data.messageDateTime DESC"),
                        m_db);

    m_dbModel->setQuery(m_query);

    setSourceModel(m_dbModel);
}

MainLogModel::~MainLogModel()
{

}

QVariant MainLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const int row = index.row();

    switch (role) {
        case MainLogModel::ContactDisplayNameRole:
        case MainLogModel::ContactIdRole:
        case MainLogModel::PersonUriRole:
        {
            const QVariant contactId = m_dbModel->record(row).value(QStringLiteral("targetContact"));
            if (role == MainLogModel::ContactIdRole) {
                return contactId;
            }

            // TODO: Cache this or something
            const KPeople::PersonData person(contactId.toString());

            if (role == MainLogModel::PersonUriRole) {
                return person.personUri();
            } else if (role == MainLogModel::ContactDisplayNameRole) {
                return person.name();
            }
        }
        case MainLogModel::AccountIdRole:
            return m_dbModel->record(row).value(QStringLiteral("accountObjectPath")).toString().mid(35);
        case MainLogModel::LastMessageDateRole:
        case MainLogModel::LastMessageTextRole:
        case MainLogModel::ConversationRole:
        case MainLogModel::HasUnreadMessagesRole:
            const QString hashKey = m_dbModel->record(row).value(QStringLiteral("accountObjectPath")).toString().mid(35)
                                  + m_dbModel->record(row).value(QStringLiteral("targetContact")).toString();

            const QHash<QString, Conversation*>::const_iterator i = m_conversations.find(hashKey);
            if (i == m_conversations.end()) {
                if (role == MainLogModel::ConversationRole) {
                    return QVariant();
                } else if (role == MainLogModel::HasUnreadMessagesRole) {
                    return false;
                } else if (role == MainLogModel::LastMessageDateRole) {
                    return m_dbModel->record(row).value(QStringLiteral("messageDateTime"));
                } else if (role == MainLogModel::LastMessageTextRole) {
                    return m_dbModel->record(row).value(QStringLiteral("message"));
                }
            } else {
                if (role == MainLogModel::ConversationRole) {
                    return QVariant::fromValue(i.value());
                } else if (role == MainLogModel::HasUnreadMessagesRole) {
                    return i.value()->hasUnreadMessages();
                } else if (role == MainLogModel::LastMessageDateRole) {
                    return i.value()->messages()->lastMessageDateTime();
                } else if (role == MainLogModel::LastMessageTextRole) {
                    return i.value()->messages()->lastMessage();
                }
            }
    }

    return QVariant();
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

    return roles;
}

void MainLogModel::startChat(const QString &accountId, const QString &contactId)
{
    const QString objectPath = TP_QT_ACCOUNT_OBJECT_PATH_BASE + QLatin1Char('/') + accountId;
    const Tp::AccountPtr account = m_accountManager->accountForObjectPath(objectPath);

    if (account.isNull()) {
        qWarning() << "Cannot get account for" << accountId;
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

    m_dbModel->clear();
    m_dbModel->setQuery(m_query);
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
        const QHash<QString, Conversation*>::const_iterator i = m_conversations.find(accountId + contactId);
        if (i == m_conversations.end()) {
            Conversation *conversation = new Conversation(this);
            setupSignals(conversation);
            m_conversations.insert(accountId + contactId, conversation);

            conversation->setAccount(account);
            conversation->setTextChannel(channel);
        } else {
            (*i)->setAccount(account);
            (*i)->setTextChannel(channel);
        }

        QModelIndex contactIndex = indexForContact(account->objectPath(), contactId);
        if (contactIndex.isValid()) {
            Q_EMIT dataChanged(contactIndex, contactIndex);
        }
    }
}

QModelIndex MainLogModel::indexForContact(const QString &accountObjectPath, const QString &contactId) const
{
    for (int i = 0; i < rowCount(); i++) {

        if (m_dbModel->record(i).value(QStringLiteral("targetContact")).toString() == contactId
            && m_dbModel->record(i).value(QStringLiteral("accountObjectPath")).toString() == accountObjectPath) {

            return createIndex(i, 0);
        }
    }

    return QModelIndex();
}

void MainLogModel::setupSignals(Conversation *conversation) const
{
    connect(conversation, &Conversation::unreadMessagesChanged, this, &MainLogModel::onConversationChanged);
    connect(conversation, &Conversation::avatarChanged, this, &MainLogModel::onConversationChanged);
    connect(conversation, &Conversation::presenceIconChanged, this, &MainLogModel::onConversationChanged);
    connect(conversation, &Conversation::titleChanged, this, &MainLogModel::onConversationChanged);
    connect(conversation, &Conversation::validityChanged, this, &MainLogModel::onConversationChanged);
}

void MainLogModel::onConversationChanged()
{
    Conversation *conversation = qobject_cast<Conversation*>(sender());
    if (!conversation || !conversation->isValid()) {
        return;
    }

    const QModelIndex index = indexForContact(conversation->account()->objectPath(), conversation->targetContact()->id());

    if (index.isValid()) {
        Q_EMIT dataChanged(index, index);
    }
}
