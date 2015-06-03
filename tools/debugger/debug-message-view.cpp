/*
    Copyright (C) 2011 Collabora Ltd. <info@collabora.com>
      @author George Kiagiadakis <george.kiagiadakis@collabora.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "debug-message-view.h"

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Constants>
#include <TelepathyQt/DebugReceiver>
#include <TelepathyQt/PendingDebugMessageList>

#include <QDebug>
#include <QAction>
#include <QFileDialog>
#include <KColorScheme>
#include <KStandardAction>
#include <KLocalizedString>
#include <KFindDialog>
#include <KService>
#include <KTextEditor/View>
#include <kfind.h>
#include <KTextEditor/Document>

#include <ctime>
#include <QDate>
#include <QPointer>
#include <QLayout>
#include <QMenu>

DebugMessageView::DebugMessageView(QWidget *parent)
    : QWidget(parent)
    , m_ready(false)
{
    KService::Ptr service = KService::serviceByDesktopPath(QString::fromLatin1("katepart.desktop"));

    if (service) {
        m_editor = qobject_cast<KTextEditor::Document*>(service->createInstance<KParts::ReadWritePart>(this));
        Q_ASSERT(m_editor && "Failed to instantiate a KatePart");
    }
    else {
        qCritical() << "Could not find kate part";
    }

    m_editor->setReadWrite(false);
    KTextEditor::View* view = m_editor->createView(this);
    setLayout(new QHBoxLayout());
    layout()->addWidget(view);
    m_editor->setHighlightingMode(QString::fromLatin1("KTp"));

    view->setContextMenu(new QMenu());
    view->contextMenu()->addAction(KStandardAction::clear(this, SLOT(clear()), this));
}

void DebugMessageView::clear()
{
    m_editor->setReadWrite(true);
    m_editor->clear();
    m_editor->setReadWrite(false);
}

void DebugMessageView::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    addDelayedMessages();
}

void DebugMessageView::addDelayedMessages()
{
    KTextEditor::Document::EditingTransaction transaction(m_editor);
    Q_FOREACH(const Tp::DebugMessage &msg, m_tmpCache) {
        appendMessage(msg);
    }
    m_tmpCache.clear();
}

DebugMessageView::~DebugMessageView()
{
    if (m_debugReceiver && m_ready) {
        //disable monitoring and do it synchronously before all the objects are destroyed
        Tp::PendingOperation *op = m_debugReceiver->setMonitoringEnabled(false);
        QEventLoop loop;
        connect(op, SIGNAL(finished(Tp::PendingOperation*)), &loop, SLOT(quit()));
        loop.exec();
    }
}

void DebugMessageView::setService(const QString &service)
{
    m_serviceName = service;
    m_serviceWatcher = new QDBusServiceWatcher(service, QDBusConnection::sessionBus(),
                                               QDBusServiceWatcher::WatchForRegistration, this);
    connect(m_serviceWatcher, SIGNAL(serviceRegistered(QString)),
            SLOT(onServiceRegistered(QString)));

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(service)) {
        onServiceRegistered(service);
    }
}


void DebugMessageView::onServiceRegistered(const QString & service)
{
    qDebug() << "Service" << service << "registered. Introspecting Debug interface...";

    m_debugReceiver = Tp::DebugReceiver::create(service);

    Tp::PendingReady *op = m_debugReceiver->becomeReady();
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onDebugReceiverReady(Tp::PendingOperation*)));
}

void DebugMessageView::onDebugReceiverInvalidated(Tp::DBusProxy *proxy,
                                                    const QString &errorName, const QString &errorMessage)
{
    Q_UNUSED(proxy);
    qDebug() << "DebugReceiver invalidated" << errorName << errorMessage;
    m_ready = false;
    m_debugReceiver.reset();
}

void DebugMessageView::onDebugReceiverReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qDebug() << "Failed to introspect Debug interface for" << m_serviceName
                 << "Error was:" << op->errorName() << "-" << op->errorMessage();
        m_debugReceiver.reset();
    } else {
        connect(m_debugReceiver.data(), SIGNAL(newDebugMessage(Tp::DebugMessage)),
                SLOT(onNewDebugMessage(Tp::DebugMessage)));

        connect(m_debugReceiver->setMonitoringEnabled(true),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onDebugReceiverMonitoringEnabled(Tp::PendingOperation*)));
    }
}

void DebugMessageView::onDebugReceiverMonitoringEnabled(Tp::PendingOperation* op)
{
    if (op->isError()) {
        qWarning() << "Failed to enable monitoring on the Debug object of" << m_serviceName
                 << "Error was:" << op->errorName() << "-" << op->errorMessage();
        m_tmpCache.clear();
        m_debugReceiver.reset();
    } else {
        connect(m_debugReceiver->fetchMessages(), SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onFetchMessagesFinished(Tp::PendingOperation*)));
    }
}

void DebugMessageView::onFetchMessagesFinished(Tp::PendingOperation* op)
{
    if (op->isError()) {
        qWarning() << "Failed to fetch messages from" << m_serviceName
                 << "Error was:" << op->errorName() << "-" << op->errorMessage();
        m_tmpCache.clear();
        m_debugReceiver.reset();
    } else {
        Tp::PendingDebugMessageList *pdml = qobject_cast<Tp::PendingDebugMessageList*>(op);
        Tp::DebugMessageList messages = pdml->result();
        messages.append(m_tmpCache); //append any messages that were received from onNewDebugMessage()
        m_tmpCache.clear();

        {
            KTextEditor::Document::EditingTransaction transaction(m_editor);
            Q_FOREACH(const Tp::DebugMessage &msg, messages) {
                appendMessage(msg);
            }
        }

        //TODO limit m_messages size

        m_ready = true;
        connect(m_debugReceiver.data(),
                SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                SLOT(onDebugReceiverInvalidated(Tp::DBusProxy*,QString,QString)));
    }
}

void DebugMessageView::onNewDebugMessage(const Tp::DebugMessage & msg)
{
    if (m_ready) {
        appendMessage(msg);
    } else {
        //cache until we are ready
        m_tmpCache.append(msg);
    }
}


//taken from empathy
static inline QString formatTimestamp(double timestamp)
{
    struct tm *tstruct;
    char time_str[32];
    int ms;
    time_t sec;

    ms = (int) ((timestamp - (int) timestamp)*1e6);
    sec = (long) timestamp;
    tstruct = std::localtime((time_t *) &sec);
    if (!std::strftime(time_str, sizeof(time_str), "%x %T", tstruct)) {
        qDebug() << "Failed to format timestamp" << timestamp;
        time_str[0] = '\0';
    }

    QString str;
    str.sprintf("%s.%d", time_str, ms);
    return str;
}

void DebugMessageView::appendMessage(const Tp::DebugMessage &msg)
{
    if ( isVisible() ) {
        QString message = QString(formatTimestamp(msg.timestamp) %
                                QLatin1Literal(" - [") % msg.domain % QLatin1Literal("] ") %
                                msg.message);
        m_editor->setReadWrite(true);
        m_editor->insertText(m_editor->documentEnd(), message + QString::fromLatin1("\n"));
        m_editor->setReadWrite(false);
    }
    else {
        m_tmpCache.append(msg);
    }
}

void DebugMessageView::saveLogFile()
{
    QUrl savedFile = QUrl::fromUserInput(QFileDialog::getSaveFileName(this, i18nc("@title:window", "Save Log")));
    m_editor->saveAs(savedFile);
}



