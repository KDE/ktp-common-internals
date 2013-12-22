/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2013  David Edmundson <davidedmundson@kde.org>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "ktp-contact-monitor.h"

#include <KTp/contact.h>
#include <KTp/core.h>

#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingReady>


KTpContactMonitor::KTpContactMonitor(const QString &localId):
    KPeople::ContactMonitor(localId)
{
    //hack till I merge that other patch
    if (localId.startsWith(QLatin1String("ktp://"))) {

        //strip leading ktp://
        QString id = localId.mid(strlen("ktp://"));
        int indexOfSlash = id.lastIndexOf(QLatin1Char('/'));
        Q_ASSERT(midPoint >= 0);
        QString accountId = id.left(indexOfSlash);
        QString contactId = id.mid(indexOfSlash+1);

        m_contact = KTp::PersistentContact::create(accountId, contactId);

        //TODO move this account manager singleton code inside KTp::PersistentContact
        //or ideally port PersistentContact to Tp::AccountFactory::proxy() and save even making an account manager
        connect(KTp::accountManager()->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)), SLOT(onAccountManagerReady()));

        connect(m_contact.data(), SIGNAL(contactChanged(KTp::ContactPtr)), SLOT(onContactChanged(KTp::ContactPtr)));
    }
}

KTpContactMonitor::~KTpContactMonitor()
{
}

void KTpContactMonitor::onAccountManagerReady()
{
    m_contact->setAccountManager(KTp::accountManager());
}


void KTpContactMonitor::onContactChanged(const KTp::ContactPtr &contact)
{
    qDebug() << "CONTACT CHAGNED";
    //The ContactPtr object has changed, connect to the new object's signals.

    connect(contact.data(), SIGNAL(presenceChanged(Tp::Presence)),
            this, SLOT(onContactDataChanged()));

    connect(contact.data(), SIGNAL(capabilitiesChanged(Tp::ContactCapabilities)),
            this, SLOT(onContactDataChanged()));

    connect(contact.data(), SIGNAL(invalidated()),
            this, SLOT(onContactInvalidated()));

    connect(contact.data(), SIGNAL(avatarDataChanged(Tp::AvatarData)),
            this, SLOT(onContactDataChanged()));

    connect(contact.data(), SIGNAL(addedToGroup(QString)),
            this, SLOT(onContactDataChanged()));

    connect(contact.data(), SIGNAL(removedFromGroup(QString)),
            this, SLOT(onContactDataChanged()));

    onContactDataChanged();
}

void KTpContactMonitor::onContactDataChanged()
{
    KTp::ContactPtr contact = m_contact->contact();
    Tp::AccountPtr account = m_contact->account();
    if (contact && account) {
        //TODO this is duplicated code from ktp-all-contacts-monitor, we can reduce this
        KABC::Addressee vcard;
        vcard.setFormattedName(contact->alias());
        vcard.setCategories(contact->groups());
        vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("contactId"), contact->id());
        vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("accountPath"), account->objectPath());
        vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("presence"), contact->presence().status());
        vcard.setPhoto(KABC::Picture(contact->avatarData().fileName));

        setContact(vcard);
    }
}


#include "ktp-contact-monitor.moc"
