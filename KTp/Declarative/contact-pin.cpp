/*
    Copyright (C) 2012 Aleix Pol <aleixpol@kde.org>

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

#include "contact-pin.h"
#include <TelepathyQt/Account>
#include <TelepathyQt/Contact>
#include "pinned-contacts-model.h"

ContactPin::ContactPin(QObject *parent)
    : QObject(parent)
    , m_model(0)
{
}

Tp::AccountPtr ContactPin::account() const
{
    return m_account;
}

KTp::ContactPtr ContactPin::contact() const
{
    return m_contact;
}

PinnedContactsModel* ContactPin::model() const
{
    return m_model;
}

bool ContactPin::isPinned() const
{
    bool ret = false;
    if (m_model && m_account && m_contact) {
        QModelIndex idx = m_model->indexForContact(m_contact);
        ret = idx.isValid();
    }
    return ret;
}

void ContactPin::toggle()
{
    Q_ASSERT(m_model && m_account && m_contact);
    m_model->setPinning(m_account, m_contact, !isPinned());
    Q_EMIT pinnedChanged();
}


void ContactPin::setAccount(const Tp::AccountPtr &account)
{
    Q_ASSERT(account);
    m_account = account;
    Q_EMIT pinnedChanged();
}

void ContactPin::setContact(const KTp::ContactPtr &contact)
{
    m_contact = contact;
    Q_EMIT pinnedChanged();
}

void ContactPin::setModel(PinnedContactsModel *model)
{
    m_model = model;
    Q_EMIT pinnedChanged();
}
