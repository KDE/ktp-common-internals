/***************************************************************************
 *   Copyright (C) 2011 by Francesco Nwokeka <francesco.nwokeka@gmail.com> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef TELEPATHY_CONTACT_LIST_H
#define TELEPATHY_CONTACT_LIST_H

#include <Plasma/Applet>
#include <Plasma/DeclarativeWidget>

#include <TelepathyQt4/Types>

class AccountsModel;

namespace Tp {
class PendingOperation;
}

class TelepathyContactList : public Plasma::Applet
{
    Q_OBJECT
public:
    TelepathyContactList(QObject *parent, const QVariantList &args);
    virtual ~TelepathyContactList();

    Q_PROPERTY(int width READ appletWidth);
    Q_PROPERTY(int height READ appletHeight);

    int appletHeight() const;     /** returns plasma applet's height */
    int appletWidth() const;      /** returns plasma applet's width */
    void init();

private slots:
    void onAccountManagerReady(Tp::PendingOperation *op);
//     QString extractAvatarPathFromNepomuk(const QString &nepomukUri);

private:
    Plasma::DeclarativeWidget *m_declarative;
    QObject *m_qmlObject;
    Tp::AccountManagerPtr m_accountManager;
    AccountsModel *m_model;
};

#endif  // TELEPATHY_CONTACT_LIST_H
