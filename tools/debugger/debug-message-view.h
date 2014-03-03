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
#ifndef DEBUG_MESSAGES_MODEL_H
#define DEBUG_MESSAGES_MODEL_H

#include <QTextEdit>
#include <TelepathyQt/Types>
#include <TelepathyQt/PendingOperation>
#include <kparts/part.h>
#include <KTextEditor/Document>


class DebugMessageView : public QWidget
{
Q_OBJECT
public:
    explicit DebugMessageView(QWidget *parent = 0);
    void setService(const QString & service);
    virtual ~DebugMessageView();
    virtual void showEvent(QShowEvent* );
    void saveLogFile();

private Q_SLOTS:
    void onServiceRegistered(const QString & service);
    void onDebugReceiverInvalidated(Tp::DBusProxy *proxy,
            const QString &errorName, const QString &errorMessage);
    void onDebugReceiverReady(Tp::PendingOperation *op);
    void onDebugReceiverMonitoringEnabled(Tp::PendingOperation *op);
    void onFetchMessagesFinished(Tp::PendingOperation *op);
    void onNewDebugMessage(const Tp::DebugMessage &msg);
    void addDelayedMessages();
    void clear();

Q_SIGNALS:
    void statusMessage(const QString& msg);

private:
    void appendMessage(const Tp::DebugMessage &msg);

    QString m_serviceName;
    Tp::DebugReceiverPtr m_debugReceiver;
    Tp::DebugMessageList m_tmpCache;
    QDBusServiceWatcher *m_serviceWatcher;
    bool m_ready;
    KTextEditor::Document* m_editor;
};

#endif // DEBUG_MESSAGES_MODEL_H
