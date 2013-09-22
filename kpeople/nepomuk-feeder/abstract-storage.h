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

private:
    Q_DISABLE_COPY(AbstractStorage);
};


#endif // Header guard

