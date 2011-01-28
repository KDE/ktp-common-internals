/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
 *   @author George Goldberg <george.goldberg@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef NEPOMUK_TELEPATHY_SERVICE_ABSTRACT_STORAGE_H
#define NEPOMUK_TELEPATHY_SERVICE_ABSTRACT_STORAGE_H

#include <QtCore/QObject>
#include <QtCore/QString>

#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Types>

/**
 * Abstract base class for all storage implementations. Primarily to ease
 * unit testing, however, this could potentially be used to replace the Nepomuk
 * storage layer with some other storage layer.
 */
class AbstractStorage : public QObject
{
    Q_OBJECT

public:
    explicit AbstractStorage(QObject *parent = 0);
    virtual ~AbstractStorage();

public Q_SLOTS:
    virtual void createAccount(const QString &path, const QString &id, const QString &protocol) = 0;
    virtual void destroyAccount(const QString &path) = 0;
    virtual void setAccountNickname(const QString &path, const QString &nickname) = 0;
    virtual void setAccountCurrentPresence(const QString &path, const Tp::SimplePresence &presence) = 0;

    virtual void createContact(const QString &path, const QString &id) = 0;
    virtual void destroyContact(const QString &path, const QString &id) = 0;
    virtual void setContactAlias(const QString &path, const QString &id, const QString &alias) = 0;
    virtual void setContactPresence(const QString &path, const QString &id, const Tp::SimplePresence &presence) = 0;
    virtual void setContactGroups(const QString &path, const QString &id, const QStringList &groups) = 0;
    virtual void setContactBlockStatus(const QString &path, const QString &id, bool blocked) = 0;
    virtual void setContactPublishState(const QString &path, const QString &id, const Tp::Contact::PresenceState &state) = 0;
    virtual void setContactSubscriptionState(const QString &path, const QString &id, const Tp::Contact::PresenceState &state) = 0;

private:
    Q_DISABLE_COPY(AbstractStorage);
};


#endif // Header guard

