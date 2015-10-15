/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
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
 *
 */

#include "log-entity.h"

using namespace KTp;

class LogEntity::Private: public QSharedData
{
  public:
    Private(Tp::HandleType entityType_, const QString &id_, const QString &alias_):
        QSharedData(),
        entityType(entityType_),
        id(id_),
        alias(alias_)
    {
    }

    Private():
        QSharedData(),
        entityType(Tp::HandleTypeNone)
    {
    }

    Private(const Private &other):
        QSharedData(other),
        entityType(other.entityType),
        id(other.id),
        alias(other.alias)
    {
    }

    ~Private()
    {
    }

    bool operator==(const Private &other)
    {
        return entityType == other.entityType
                && id == other.id
                && alias == other.alias;
    }

    Tp::HandleType entityType;
    QString id;
    QString alias;
};

LogEntity::LogEntity(Tp::HandleType entityType, const QString& id, const QString& alias):
    d(new Private(entityType, id, alias))
{
}

LogEntity::LogEntity():
    d(new Private)
{
}

LogEntity::LogEntity(const LogEntity& other):
    d(other.d)
{
}

LogEntity::~LogEntity()
{
}

LogEntity& LogEntity::operator=(const LogEntity& other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

bool LogEntity::operator==(const LogEntity& other)
{
    return *d == *other.d;
}

bool LogEntity::operator!=(const LogEntity& other)
{
    return !(operator==(other));
}

bool LogEntity::isValid() const
{
    return d->entityType != Tp::HandleTypeNone
            && !d->id.isEmpty()
            && !d->alias.isEmpty();
}

Tp::HandleType LogEntity::entityType() const
{
    return d->entityType;
}

QString LogEntity::id() const
{
    return d->id;
}

QString LogEntity::alias() const
{
    return d->alias;
}
