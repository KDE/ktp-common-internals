/*
    Copyright (C) 2011 Collabora Ltd. <info@collabora.com>
      @author George Kiagiadakis <george.kiagiadakis@collabora.com>

    Copyright (C) 2016 Alexandr Akulich <akulichalexander@gmail.com>

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
#include "telepathy-process.h"

#include <TelepathyQt/Constants>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingDebugMessageList>

TelepathyProcess::TelepathyProcess(QObject *parent) :
    QObject(parent),
    m_ready(false)
{
}

TelepathyProcess::~TelepathyProcess()
{
    if (m_debugReceiver && m_ready) {
        //disable monitoring and do it synchronously before all the objects are destroyed
        Tp::PendingOperation *op = m_debugReceiver->setMonitoringEnabled(false);
        QEventLoop loop;
        connect(op, SIGNAL(finished(Tp::PendingOperation*)), &loop, SLOT(quit()));
        loop.exec();
    }
}

void TelepathyProcess::setOwnerId(const QString &owner)
{
    m_owner = owner;
    m_debugReceiver = Tp::DebugReceiver::create(owner);

    Tp::PendingReady *op = m_debugReceiver->becomeReady();
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onDebugReceiverReady(Tp::PendingOperation*)));
}

void TelepathyProcess::onDebugReceiverInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage)
{
    Q_UNUSED(proxy);
    qDebug() << "DebugReceiver invalidated" << errorName << errorMessage;
    m_debugReceiver.reset();
    m_ready = false;
}

void TelepathyProcess::onDebugReceiverReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qDebug() << "Failed to introspect Debug interface for" << m_owner
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

void TelepathyProcess::onDebugReceiverMonitoringEnabled(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qWarning() << "Failed to enable monitoring on the Debug object of" << m_owner
                   << "Error was:" << op->errorName() << "-" << op->errorMessage();
        m_tmpCache.clear();
        m_debugReceiver.reset();
    } else {
        connect(m_debugReceiver->fetchMessages(), SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onFetchMessagesFinished(Tp::PendingOperation*)));
    }
}

void TelepathyProcess::onFetchMessagesFinished(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qWarning() << "Failed to fetch messages from" << m_owner
                   << "Error was:" << op->errorName() << "-" << op->errorMessage();
        m_tmpCache.clear();
        m_debugReceiver.reset();
    } else {
        Tp::PendingDebugMessageList *pendingMessages = qobject_cast<Tp::PendingDebugMessageList*>(op);
        Tp::DebugMessageList messages = pendingMessages->result();
        messages.append(m_tmpCache); //append any messages that were received from onNewDebugMessage()
        m_tmpCache.clear();

        Q_FOREACH(const Tp::DebugMessage &msg, messages) {
            appendMessage(msg);
        }

        m_ready = true;
        connect(m_debugReceiver.data(),
                SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                SLOT(onDebugReceiverInvalidated(Tp::DBusProxy*,QString,QString)));
    }
}

void TelepathyProcess::onNewDebugMessage(const Tp::DebugMessage &msg)
{
    if (m_ready) {
        appendMessage(msg);
    } else {
        //cache until we are ready
        m_tmpCache.append(msg);
    }
}

void TelepathyProcess::appendMessage(const Tp::DebugMessage &msg)
{
    Q_EMIT newDebugMessage(msg);
}
