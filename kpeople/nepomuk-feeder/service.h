/*
 * This file is part of telepathy-nepomuk-service
 *
 * Copyright (C) 2010 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
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

#ifndef TELEPATHY_NEPOMUK_SERVICE_SERVICE_H
#define TELEPATHY_NEPOMUK_SERVICE_SERVICE_H

#include <Nepomuk2/Service>

#include <QtCore/QVariantList>

class Controller;

class TelepathyService : public Nepomuk2::Service
{
    Q_OBJECT

public:
    TelepathyService(QObject* parent, const QVariantList&);
    ~TelepathyService();

public Q_SLOTS:
    void onStorageInitialisationFailed();

private:
    Controller *m_controller;
};


#endif // TELEPATHY_NEPOMUK_SERVICE_SERVICE_H

