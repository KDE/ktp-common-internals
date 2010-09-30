/*
 * This file is part of nepomuktelepathyservice
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

#ifndef TELEPATHY_INTEGRATION_DAEMON_TELEPATHYCONTACT_H
#define TELEPATHY_INTEGRATION_DAEMON_TELEPATHYCONTACT_H

#include "imaccount.h"
#include "personcontact.h"

#include <QtCore/QObject>

#include <TelepathyQt4/Connection>
#include <TelepathyQt4/Contact>

class TelepathyAccount;

/**
 * This class takes care of one contact in the roster of a given telepathy account on the AM.
 */
class TelepathyContact : public QObject
{
    Q_OBJECT

public:
    explicit TelepathyContact(Tp::ContactPtr contact,
                              Tp::ConnectionPtr connection,
                              Nepomuk::IMAccount accountResource,
                              TelepathyAccount *parent = 0);
    ~TelepathyContact();

    QString avatarToken() const;
    void setAvatar(const QString &token, const QByteArray &data);

Q_SIGNALS:
    void contactDestroyed(const Tp::ContactPtr &contact);

private Q_SLOTS:
    void onAliasChanged(const QString &alias);
    void onPresenceChanged(const QString &status, uint type, const QString &message);
    void onAddedToGroup(const QString &group);
    void onRemovedFromGroup(const QString &group);
    void onBlockStatusChanged(bool blocked);
    void onPublishStateChanged(Tp::Contact::PresenceState state);
    void onSubscriptionStateChanged(Tp::Contact::PresenceState state);
    void onCapabilitiesChanged(Tp::ContactCapabilities*);

private:
    Q_DISABLE_COPY(TelepathyContact);

    void doNepomukSetup();
    QList< Nepomuk::Resource > processCapability(const QUrl &capability,
                                                 bool isSupported,
                                                 const QList< Nepomuk::Resource >& resources);

    TelepathyAccount *m_parent;
    Tp::ContactPtr m_contact;
    Tp::ConnectionPtr m_connection;

    Nepomuk::IMAccount m_accountResource;
    Nepomuk::IMAccount m_contactIMAccountResource;
    Nepomuk::PersonContact m_contactPersonContactResource;
};


#endif // Header guard

