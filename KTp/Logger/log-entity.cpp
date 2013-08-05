/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "log-entity.h"

using namespace KTp;

class LogEntity::Private: public QSharedData
{
  public:
    Private(const QString &id_, const QString &alias_):
        QSharedData(),
        id(id_),
        alias(alias_)
    {
    }

    Private():
        QSharedData()
    {
    }

    Private(const Private &other):
        QSharedData(other),
        id(other.id),
        alias(other.alias)
    {
    }

    QString id;
    QString alias;
};

LogEntity::LogEntity(const QString& id, const QString& alias):
    d(new Private(id, alias))
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
    if (d != other.d) {
        d = other.d;
    }

    return *this;
}

bool LogEntity::operator==(const LogEntity& other)
{
    return d->id == other.d->id && d->alias == other.d->alias;
}

bool LogEntity::isValid()
{
    return !d->id.isEmpty() && !d->alias.isEmpty();
}

QString LogEntity::id() const
{
    return d->id;
}

QString LogEntity::alias() const
{
    return d->alias;
}
