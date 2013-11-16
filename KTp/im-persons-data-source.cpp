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
    QString createUri(const Tp::ContactPtr &contact) const;
    KABC::Addressee contactToAddressee(const Tp::ContactPtr &contact) const;
    QHash<QString, KTp::ContactPtr> m_contacts;
};

KTpAllContacts::KTpAllContacts()
{
    Tp::registerTypes();

    connect(KTp::accountManager()->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

KTpAllContacts::~KTpAllContacts()
{
}

QString KTpAllContacts::createUri(const Tp::ContactPtr &contact) const
{
    return QLatin1String("ktp://") + contact->id();
}

void KTpAllContacts::onAccountManagerReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Failed to initialize AccountManager:" << op->errorName();
        kWarning() << op->errorMessage();

        return;
    }

    kDebug() << "Account manager ready";

    connect(KTp::contactManager(), SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts)),
            this, SLOT(onAllKnownContactsChanged(Tp::Contacts,Tp::Contacts)));

    onAllKnownContactsChanged(KTp::contactManager()->allKnownContacts(), Tp::Contacts());
}

void KTpAllContacts::onAllKnownContactsChanged(const Tp::Contacts &contactsAdded, const Tp::Contacts &contactsRemoved)
{
    if (!m_contacts.isEmpty()) {
        Q_FOREACH (const Tp::ContactPtr &contact, contactsRemoved) {
            m_contacts.remove(contact->id());
            Q_EMIT contactRemoved(createUri(contact));
        }
    }

    Q_FOREACH (const Tp::ContactPtr &contact, contactsAdded) {
        KTp::ContactPtr ktpContact = KTp::ContactPtr::qObjectCast(contact);
        m_contacts.insert(contact->id(), ktpContact);
        Q_EMIT contactAdded(createUri(ktpContact), contactToAddressee(ktpContact));

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
    const Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));
    Q_EMIT contactChanged(createUri(contact), contactToAddressee(contact));
}

void KTpAllContacts::onContactInvalidated()
{
    const Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));
    Q_EMIT contactChanged(createUri(contact), contactToAddressee(contact));
    m_contacts.remove(contact->id());
}

KABC::Addressee::Map KTpAllContacts::contacts()
{
    KABC::Addressee::Map contactMap;
    Q_FOREACH(const Tp::ContactPtr &contact, m_contacts.values()) {
        contactMap.insert(createUri(contact), contactToAddressee(contact));
    }
    kDebug() << contactMap.keys().size();
    return contactMap;
}

KABC::Addressee KTpAllContacts::contactToAddressee(const Tp::ContactPtr &contact) const
{
    KABC::Addressee vcard;
    Tp::AccountPtr account = KTp::contactManager()->accountForContact(contact);
    if (contact && account) {
        vcard.setFormattedName(contact->alias());
        vcard.setCategories(contact->groups());
        vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("contactId"), contact->id());
        vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("accountPath"), account->objectPath());
        vcard.insertCustom(QLatin1String("telepathy"), QLatin1String("presence"), contact->presence().status());
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
