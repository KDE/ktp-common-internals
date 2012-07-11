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
#include "debug-messages-model.h"

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Constants>

#include <KDebug>
#include <KIcon>
#include <KColorScheme>

#include <ctime>

DebugMessagesModel::DebugMessagesModel(const QString & service, QObject *parent)
    : QAbstractListModel(parent),
      m_serviceName(service),
      m_ready(false)
{
    m_serviceWatcher = new QDBusServiceWatcher(service, QDBusConnection::sessionBus(),
                                               QDBusServiceWatcher::WatchForRegistration, this);
    connect(m_serviceWatcher, SIGNAL(serviceRegistered(QString)),
            SLOT(onServiceRegistered(QString)));

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(service)) {
        onServiceRegistered(service);
    }
}

DebugMessagesModel::~DebugMessagesModel()
{
    if (m_debugReceiver && m_ready) {
        //disable monitoring and do it synchronously before all the objects are destroyed
        Tp::PendingOperation *op = m_debugReceiver->setMonitoringEnabled(false);
        QEventLoop loop;
        connect(op, SIGNAL(finished(Tp::PendingOperation*)), &loop, SLOT(quit()));
        loop.exec();
    }
}

void DebugMessagesModel::onServiceRegistered(const QString & service)
{
    kDebug() << "Service" << service << "registered. Introspecting Debug interface...";

    m_debugReceiver = Tp::DebugReceiver::create(service);

    Tp::PendingReady *op = m_debugReceiver->becomeReady();
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onDebugReceiverReady(Tp::PendingOperation*)));
}

void DebugMessagesModel::onDebugReceiverInvalidated(Tp::DBusProxy *proxy,
                                                    const QString &errorName, const QString &errorMessage)
{
    Q_UNUSED(proxy);
    kDebug() << "DebugReceiver invalidated" << errorName << errorMessage;
    m_ready = false;
    m_debugReceiver.reset();
}

void DebugMessagesModel::onDebugReceiverReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kDebug() << "Failed to introspect Debug interface for" << m_serviceName
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

void DebugMessagesModel::onDebugReceiverMonitoringEnabled(Tp::PendingOperation* op)
{
    if (op->isError()) {
        kError() << "Failed to enable monitoring on the Debug object of" << m_serviceName
                 << "Error was:" << op->errorName() << "-" << op->errorMessage();
        m_tmpCache.clear();
        m_debugReceiver.reset();
    } else {
        connect(m_debugReceiver->fetchMessages(), SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onFetchMessagesFinished(Tp::PendingOperation*)));
    }
}

void DebugMessagesModel::onFetchMessagesFinished(Tp::PendingOperation* op)
{
    if (op->isError()) {
        kError() << "Failed to fetch messages from" << m_serviceName
                 << "Error was:" << op->errorName() << "-" << op->errorMessage();
        m_tmpCache.clear();
        m_debugReceiver.reset();
    } else {
        Tp::PendingDebugMessageList *pdml = qobject_cast<Tp::PendingDebugMessageList*>(op);
        Tp::DebugMessageList messages = pdml->result();
        messages.append(m_tmpCache); //append any messages that were received from onNewDebugMessage()
        m_tmpCache.clear();

        beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size() + messages.size());
        m_messages.append(messages);
        endInsertRows();

        //TODO limit m_messages size

        m_ready = true;
        connect(m_debugReceiver.data(),
                SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                SLOT(onDebugReceiverInvalidated(Tp::DBusProxy*,QString,QString)));
    }
}

void DebugMessagesModel::onNewDebugMessage(const Tp::DebugMessage & msg)
{
    if (m_ready) {
        beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
        m_messages.append(msg);
        endInsertRows();

        //TODO limit m_messages size
    } else {
        //cache until we are ready
        m_tmpCache.append(msg);
    }
}

//taken from empathy
static QString formatTimestamp(double timestamp)
{
    struct tm *tstruct;
    char time_str[32];
    int ms;
    time_t sec;

    ms = (int) ((timestamp - (int) timestamp)*1e6);
    sec = (long) timestamp;
    tstruct = std::localtime((time_t *) &sec);
    if (!std::strftime(time_str, sizeof(time_str), "%x %T", tstruct)) {
        kDebug() << "Failed to format timestamp" << timestamp;
        time_str[0] = '\0';
    }

    QString str;
    str.sprintf("%s.%d", time_str, ms);
    return str;
}

QVariant DebugMessagesModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    KColorScheme scheme(QPalette::Active, KColorScheme::Window);


    switch(role) {
    case Qt::DisplayRole:
        return QString(formatTimestamp(m_messages[index.row()].timestamp) %
                       QLatin1Literal(" - [") % m_messages[index.row()].domain % QLatin1Literal("] ") %
                       m_messages[index.row()].message);
    case Qt::DecorationRole:
        switch(m_messages[index.row()].level) {
        case Tp::DebugLevelError:
            return KIcon(QLatin1String("dialog-error"));
        case Tp::DebugLevelCritical:
            return KIcon(QLatin1String("dialog-error"));
        case Tp::DebugLevelWarning:
            return KIcon(QLatin1String("dialog-warning"));
        case Tp::DebugLevelMessage:
            return KIcon(QLatin1String("dialog-info"));
        case Tp::DebugLevelInfo:
            return KIcon(QLatin1String("dialog-info"));
        case Tp::DebugLevelDebug:
            return KIcon(QLatin1String("tools-report-bug"));
        default:
            return QVariant();
        }
    case Qt::ForegroundRole:
        switch(m_messages[index.row()].level) {
        case Tp::DebugLevelError:
            /** Fall Through*/
        case Tp::DebugLevelCritical:
            /** Fall Through*/
        case Tp::DebugLevelWarning:
            return scheme.foreground(KColorScheme::NegativeText);
        case Tp::DebugLevelMessage:
            /** Fall Through*/
        case Tp::DebugLevelInfo:
            /** Fall Through*/
        case Tp::DebugLevelDebug:
            /** Fall Through*/
        default:
            return QVariant();
        }
    case MessageRole:
        return m_messages[index.row()].message;
    case TimestampRole:
        return m_messages[index.row()].timestamp;
    case LevelRole:
        return m_messages[index.row()].level;
    case DomainRole:
        return m_messages[index.row()].domain;
    case ServiceRole:
        return m_serviceName;
    default:
        return QVariant();
    }
}

int DebugMessagesModel::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid()) {
        return m_messages.size();
    } else {
        return 0;
    }
}

#include "moc_debug-messages-model.cpp"
