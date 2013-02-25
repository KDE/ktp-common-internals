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

#ifndef CONTACTPIN_H
#define CONTACTPIN_H

#include <QObject>
#include "KTp/types.h"

class PinnedContactsModel;

class ContactPin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(KTp::ContactPtr contact READ contact WRITE setContact)
    Q_PROPERTY(Tp::AccountPtr account READ account WRITE setAccount)
    Q_PROPERTY(PinnedContactsModel *model READ model WRITE setModel)
    Q_PROPERTY(bool pinned READ isPinned NOTIFY pinnedChanged)

  public:
    explicit ContactPin(QObject *parent = 0);

    KTp::ContactPtr contact() const;
    Tp::AccountPtr account() const;
    PinnedContactsModel* model() const;
    bool isPinned() const;

    void setContact(const KTp::ContactPtr &contact);
    void setAccount(const Tp::AccountPtr &account);
    void setModel(PinnedContactsModel *model);

    Q_SCRIPTABLE void toggle();

  Q_SIGNALS:
    void pinnedChanged();

  private:
    PinnedContactsModel *m_model;
    KTp::ContactPtr m_contact;
    Tp::AccountPtr m_account;
};

#endif // CONTACTPIN_H
