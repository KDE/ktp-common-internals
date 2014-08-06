/*
    Copyright 2014  Nilesh Suthar <nileshsuthar@live.in>

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

#include "kpeople_chat_plugin.h"
#include "chatlistviewdelegate.h"

#include <QLabel>
#include <QDebug>
#include <QListView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <KLocalizedString>
#include <KGlobal>
#include <KABC/Addressee>
#include <KPluginFactory>
#include <KLocale>

#include <KTp/core.h>
#include <KTp/Logger/log-manager.h>
#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/pending-logger-dates.h>
#include <KTp/Logger/pending-logger-logs.h>
#include <KTp/message.h>
#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>

#define TP_ACCOUNT_OBJECT_PATH_BASE "/org/freedesktop/Telepathy/Account/"

K_PLUGIN_FACTORY(KpeopleChatFactory, registerPlugin<ChatWidgetFactory>();)
K_EXPORT_PLUGIN(KpeopleChatFactory("kpeople_chat_plugin", "ktp-common-internals"))

ChatWidgetFactory::ChatWidgetFactory(QObject *parent, const QVariantList &args): AbstractFieldWidgetFactory(parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(args);
    m_model = new QStandardItemModel();
}

QWidget *ChatWidgetFactory::createDetailsWidget(const KABC::Addressee &person, const KABC::AddresseeList &contacts, QWidget *parent) const
{
    Q_UNUSED(contacts);
    QWidget *widget = new QWidget(parent);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(widget);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFixedHeight(widget->height());

    QVBoxLayout *layout = new QVBoxLayout(widget);
    QListView *chatlistView = new QListView();
    ChatListviewDelegate *chatListViewDelegate = new ChatListviewDelegate(chatlistView);
    chatlistView->setItemDelegate(chatListViewDelegate);
    chatlistView->setModel(m_model);
    layout->setContentsMargins(0, 0, 0, 0);

    if (person.custom(QLatin1String("telepathy"), QLatin1String("accountPath")).isEmpty()) {
        layout->addWidget(new QLabel(QLatin1String("Chat for current contact is not supported")));
    } else {
        KTp::LogManager *logManager = KTp::LogManager::instance();
        logManager->setAccountManager(KTp::accountManager());
        KTp::LogEntity logEntity(Tp::HandleTypeContact, person.custom(QLatin1String("telepathy"), QLatin1String("contactId")));

        Tp::AccountPtr account;

        if (person.custom(QLatin1String("telepathy"), QLatin1String("accountPath")).contains(QLatin1String(TP_ACCOUNT_OBJECT_PATH_BASE))) {
            account = KTp::accountManager().data()->accountForObjectPath(person.custom(QLatin1String("telepathy"), QLatin1String("accountPath")));
        } else {
            account = KTp::accountManager().data()->accountForObjectPath(QLatin1String(TP_ACCOUNT_OBJECT_PATH_BASE) + person.custom(QLatin1String("telepathy"), QLatin1String("accountPath")));
        }

        if (account.isNull()) {
            qDebug() << "Error Occoured Account is not supposed to be null";
        } else {
            if (logManager->logsExist(account, logEntity)) {
                connect(logManager->queryDates(account, logEntity), SIGNAL(finished(KTp::PendingLoggerOperation*)), SLOT(onPendingDates(KTp::PendingLoggerOperation*)));
            } else {
                layout->addWidget(new QLabel(QLatin1String("Chat for current contact is not available")));
            }
        }
    }

    layout->addWidget(chatlistView);
    widget->setLayout(layout);

    return scrollArea;
}
void ChatWidgetFactory::onPendingDates(KTp::PendingLoggerOperation *pendingOperation)
{

    KTp::PendingLoggerDates *pd = qobject_cast<KTp::PendingLoggerDates *>(pendingOperation);
    QList<QDate> dates = pd->dates();
    if (dates.isEmpty()) {
        qDebug() << "No messages";
        return;
    }
    //Return atmost 5 logs previous logs
    int numberOfLogs = 5;
    if (dates.count() <= numberOfLogs) {
        Q_FOREACH (QDate date , dates) {
            KTp::PendingLoggerLogs *log = KTp::LogManager::instance()->queryLogs(pd->account(), pd->entity(), date);
            connect(log, SIGNAL(finished(KTp::PendingLoggerOperation*)), this, SLOT(onEventsFinished(KTp::PendingLoggerOperation*)));
        }
    } else {
        for (int i = numberOfLogs; i > 0; i--) {
            KTp::PendingLoggerLogs *log = KTp::LogManager::instance()->queryLogs(pd->account(), pd->entity(), dates[dates.count() - i]);
            connect(log, SIGNAL(finished(KTp::PendingLoggerOperation*)), this, SLOT(onEventsFinished(KTp::PendingLoggerOperation*)));
        }
    }
}

void ChatWidgetFactory::onEventsFinished(KTp::PendingLoggerOperation *pendingOperation)
{
    KTp::PendingLoggerLogs *logs = qobject_cast<KTp::PendingLoggerLogs *>(pendingOperation);
    if (logs->hasError()) {
        qDebug() << "Failed to fetch error:" << logs->error();
        return;
    }
    QStringList queuedMessageTokens;
    QList<KTp::LogMessage> messageList = logs->logs();

    Q_FOREACH (KTp::LogMessage message, messageList) {
        if (message.direction() == KTp::Message::RemoteToLocal) {
            QStandardItem *messageRow = new QStandardItem();
            messageRow->setData(message.senderAlias(), ChatListviewDelegate::senderAliasRole);
            messageRow->setData(message.mainMessagePart(), ChatListviewDelegate::messageRole);
            messageRow->setData(KGlobal::locale()->formatDateTime(message.time(), KLocale::FancyShortDate), ChatListviewDelegate::messageTimeRole);
            m_model->appendRow(messageRow);
        } else {
            QStandardItem *messageRow = new QStandardItem();
            messageRow->setData(QLatin1String("Me"), ChatListviewDelegate::senderAliasRole);
            messageRow->setData(message.mainMessagePart(), ChatListviewDelegate::messageRole);
            messageRow->setData(KGlobal::locale()->formatDateTime(message.time(), KLocale::FancyShortDate), ChatListviewDelegate::messageTimeRole);
            m_model->appendRow(messageRow);
        }
    }
}

QString ChatWidgetFactory::label() const
{
    return i18n("Chat");
}

int ChatWidgetFactory::sortWeight() const
{
    return 0;
}

#include "kpeople_chat_plugin.moc"
