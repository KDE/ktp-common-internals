/*
    Copyright (C) 2013  Martin Klapetek <mklapetek@kde.org>

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


#include "im-persons-data-source.h"

#include <KPeople/PersonsModel>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Presence>

#include "KTp/contact-factory.h"
#include "KTp/global-contact-manager.h"
#include "KTp/types.h"

#include <KDebug>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Nepomuk2/ResourceManager>

using namespace KPeople;


class KTpAllContacts : public AllContactsMonitor
{
    Q_OBJECT
public:
    KTpAllContacts();
    ~KTpAllContacts();
    virtual KABC::Addressee::Map contacts();

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onContactChanged();
    void onContactInvalidated();
    void onAllKnownContactsChanged(const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved);

private:
    KABC::Addressee contactToAddressee(const QString &contactId) const;

    KTp::GlobalContactManager *m_contactManager;
    Tp::AccountManagerPtr m_accountManager;
    QHash<QString, KTp::ContactPtr> m_contacts;
};

KTpAllContacts::KTpAllContacts()
{
    Tp::registerTypes();

    m_accountManager = KTp::accountManager();
    connect(m_accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

KTpAllContacts::~KTpAllContacts()
{
}

void KTpAllContacts::onAccountManagerReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Failed to initialize AccountManager:" << op->errorName();
        kWarning() << op->errorMessage();

        return;
    }

    kDebug() << "Account manager ready";

    m_contactManager = new KTp::GlobalContactManager(m_accountManager, this);
    connect(m_contactManager, SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts)),
            this, SLOT(onAllKnownContactsChanged(Tp::Contacts,Tp::Contacts)));

    onAllKnownContactsChanged(m_contactManager->allKnownContacts(), Tp::Contacts());
}

void KTpAllContacts::onAllKnownContactsChanged(const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved)
{
    if (!m_contacts.isEmpty()) {
        Q_FOREACH (const Tp::ContactPtr &contact, contactsRemoved) {
            m_contacts.remove(contact->id());
            Q_EMIT contactRemoved(contact->id());
        }
    }

    Q_FOREACH (const Tp::ContactPtr &contact, contactsAdded) {
        KTp::ContactPtr ktpContact = KTp::ContactPtr::qObjectCast(contact);
        m_contacts.insert(contact->id(), ktpContact);
        QString contactId = contact->id();
        Q_EMIT contactAdded(contactId, contactToAddressee(contactId));

        connect(ktpContact.data(), SIGNAL(presenceChanged(Tp::Presence)),
                this, SLOT(onContactChanged()));

        connect(ktpContact.data(), SIGNAL(capabilitiesChanged(Tp::ContactCapabilities)),
                this, SLOT(onContactChanged()));

        connect(ktpContact.data(), SIGNAL(invalidated()),
                this, SLOT(onContactInvalidated()));
    }
}

void KTpAllContacts::onContactChanged()
{
    QString id = qobject_cast<Tp::Contact*>(sender())->id();

    Q_EMIT contactChanged(id, contactToAddressee(id));
}

void KTpAllContacts::onContactInvalidated()
{
    QString id = qobject_cast<Tp::Contact*>(sender())->id();

    m_contacts.remove(id);

    Q_EMIT contactChanged(id, contactToAddressee(id));
}

KABC::Addressee::Map KTpAllContacts::contacts()
{
    KABC::Addressee::Map contactMap;
    Q_FOREACH(const QString &key, m_contacts.keys()) {
        contactMap.insert(key, contactToAddressee(key));
    }
    kDebug() << contactMap.keys().size();
    return contactMap;
}

KABC::Addressee KTpAllContacts::contactToAddressee(const QString &contactId) const
{
    KABC::Addressee vcard;

    qDebug() << "running ktp datasource" << contactId;
    KTp::ContactPtr contact = m_contacts[contactId];
    if (contact) {
        vcard.setFormattedName(contact->alias());
        vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("contactId"), contact->id());
        vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("presence"), contact->presence().status());
        //         vcard.insertCustom("telepathy", "accountId", contact->id());
        vcard.setPhoto(KABC::Picture(contact->avatarData().fileName));
    }
    return vcard;
}

IMPersonsDataSource::IMPersonsDataSource(QObject *parent, const QVariantList &args)
    : BasePersonsDataSource(parent)
{
    Q_UNUSED(args);
}

IMPersonsDataSource::~IMPersonsDataSource()
{
}

AllContactsMonitor* IMPersonsDataSource::createAllContactsMonitor()
{
    return new KTpAllContacts();
}

#include "im-persons-data-source.moc"
