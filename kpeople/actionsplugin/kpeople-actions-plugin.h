/*
    Copyright (C) 2013  David Edmundson <davidedmundson@kde.org>

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


#ifndef IM_PLUGIN_H
#define IM_PLUGIN_H

#include <KPeopleBackend/AbstractPersonAction>
#include <TelepathyQt/Connection>

class KPeopleActionsPlugin : public KPeople::AbstractPersonAction
{
    Q_OBJECT
public:
    KPeopleActionsPlugin(QObject *parent, const QVariantList &args);
    virtual QList< QAction* > actionsForPerson(const KPeople::PersonData &data,
                                               QObject *parent) const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onActionTriggered();
    void onConnectAndActionTriggered();
    void onAccountConnectionStatusChanged(Tp::ConnectionStatus status);
};

#endif // IM_PLUGIN_H
