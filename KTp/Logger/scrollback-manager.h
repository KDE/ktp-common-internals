/*
    Copyright (C) 2011  Dominik Schmidt <kde@dominik-schmidt.de>
    Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>

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


#ifndef SCROLLBACKMANAGER_H
#define SCROLLBACKMANAGER_H

#include <KTp/message.h>

namespace KTp {
class PendingLoggerOperation;
}

class KTPCOMMONINTERNALS_EXPORT ScrollbackManager : public QObject
{
    Q_OBJECT

public:
    explicit ScrollbackManager(QObject *parent = 0);
    virtual ~ScrollbackManager();

    bool exists() const;

    void setTextChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &textChannel);

    /**
     * This is an alternative for setTextChannel() to allow for offline scrollback
     *
     * @param account The account of the contact
     * @param contactId id of the contact for which the screollback is requested
     */
    void setAccountAndContact(const Tp::AccountPtr &account, const QString &contactId, const QString &contactAlias = QString());

    /**
     * Sets amount of messages to be fetched via @p fetchScrollback()
     */
    void setScrollbackLength(int n);

    int scrollbackLength() const;

    /**
     * Fetches last N message,s as set via setFetchAmount()
     */
    void fetchScrollback();

    /**
     * Fetches last @p n messages
     * If @p fromMessageToken is specified, it fetches last @p n messages
     * from the message with the given token
     */
    void fetchHistory(int n, const QString &fromMessageToken = QString());

Q_SIGNALS:
    void fetched(const QList<KTp::Message> &messages);

private Q_SLOTS:
    void onDatesFinished(KTp::PendingLoggerOperation *po);
    void onEventsFinished(KTp::PendingLoggerOperation *po);

private:
    class Private;
    Private * const d;
};

#endif // SCROLLBACKMANAGER_H
