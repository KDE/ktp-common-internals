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

#ifndef KTP_LOGENTITY_H
#define KTP_LOGENTITY_H

#include <QSharedDataPointer>
#include <QString>
#include <QMetaType>

#include <KTp/ktp-export.h>

namespace KTp {

class KTP_EXPORT LogEntity
{
  public:
    enum EntityType {
        EntityTypeInvalid,
        EntityTypeContact,
        EntityTypeRoom
    };

    explicit LogEntity();
    LogEntity(EntityType entityType, const QString &id,
              const QString &alias = QString());
    LogEntity(const KTp::LogEntity &other);
    ~LogEntity();

    KTp::LogEntity& operator=(const KTp::LogEntity &other);
    bool operator==(const KTp::LogEntity &other);

    bool isValid() const;
    QString id() const;
    QString alias() const;
    EntityType entityType() const;

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

Q_DECLARE_METATYPE(KTp::LogEntity)

#endif // KTP_LOGENTITY_H
