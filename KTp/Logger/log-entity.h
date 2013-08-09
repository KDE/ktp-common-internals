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

/**
 * @brief LogEntity represents a single contact or chat room
 */
class KTP_EXPORT LogEntity
{
  public:
    enum EntityType {
        EntityTypeInvalid,
        EntityTypeContact,
        EntityTypeRoom
    };

    /**
     * Constructs an invalid LogEntity
     */
    explicit LogEntity();

    /**
     * Constructs a valid entity
     *
     * @param entityType Whether the entity represents a contact or a chat root
     * @param id ID of the contact or chat root
     * @param alias Optional alias (username) of the contact or chat root
     */
    LogEntity(EntityType entityType, const QString &id,
              const QString &alias = QString());

    /**
     * Copy constructor
     */
    LogEntity(const KTp::LogEntity &other);

    /**
     * Destructor
     */
    ~LogEntity();

    /**
     * Assignment operator
     */
    KTp::LogEntity& operator=(const KTp::LogEntity &other);

    /**
     * Compare operator
     */
    bool operator==(const KTp::LogEntity &other);

    /**
     * Returns whether this entity is valid (i.e. whether entity type is valid and
     * whether id is not empty)
     */
    bool isValid() const;

    /**
     * Returns ID of contact or chat room that this entity represents
     */
    QString id() const;

    /**
     * Returns username of the contact or name of the chat room this entity represents
     * or an empty string if none was set.
     */
    QString alias() const;

    /**
     * Returns whether this entity represents a contact or a chat root
     */
    EntityType entityType() const;

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

Q_DECLARE_METATYPE(KTp::LogEntity)

#endif // KTP_LOGENTITY_H
