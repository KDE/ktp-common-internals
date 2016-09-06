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

#ifndef TELEPATHY_PROCESS_H
#define TELEPATHY_PROCESS_H

#include <QObject>

#include <TelepathyQt/DebugReceiver>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/Types>

class TelepathyProcess : public QObject
{
    Q_OBJECT
public:
    explicit TelepathyProcess(QObject *parent = nullptr);
    ~TelepathyProcess();

    QString ownerId() const Q_REQUIRED_RESULT { return m_owner; }
    void setOwnerId(const QString &owner);

Q_SIGNALS:
    void newDebugMessage(const Tp::DebugMessage &message);

protected Q_SLOTS:
    void onDebugReceiverInvalidated(Tp::DBusProxy *proxy,
            const QString &errorName, const QString &errorMessage);
    void onDebugReceiverReady(Tp::PendingOperation *op);
    void onDebugReceiverMonitoringEnabled(Tp::PendingOperation *op);
    void onFetchMessagesFinished(Tp::PendingOperation *op);
    void onNewDebugMessage(const Tp::DebugMessage &msg);

protected:
    void appendMessage(const Tp::DebugMessage &msg);

private:
    QString m_owner;
    Tp::DebugReceiverPtr m_debugReceiver;
    Tp::DebugMessageList m_tmpCache;
    bool m_ready;
};

#endif // TELEPATHY_PROCESS_H
