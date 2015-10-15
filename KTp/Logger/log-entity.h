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

#ifndef KTP_LOGENTITY_H
#define KTP_LOGENTITY_H

#include <QSharedDataPointer>
#include <QString>
#include <QMetaType>

#include <KTp/ktpcommoninternals_export.h>

#include <TelepathyQt/Constants>

namespace KTp {

/**
 * @brief LogEntity represents a single contact or chat room
 *
 * @since 0.7
 * @author Daniel Vrátil <dvratil@redhat.com>
 */
class KTPCOMMONINTERNALS_EXPORT LogEntity
{
  public:
    /**
     * Constructs an invalid LogEntity.
     */
    explicit LogEntity();

    /**
     * Constructs a valid entity.
     *
     * @param entityType Whether the entity represents a contact or a chat room
     * @param id ID of the contact or chat room
     * @param alias Optional alias (username) of the contact or chat room
     */
    LogEntity(Tp::HandleType entityType, const QString &id,
              const QString &alias = QString());

    /**
     * Copy constructor.
     */
    LogEntity(const KTp::LogEntity &other);

    /**
     * Destructor.
     */
    ~LogEntity();

    /**
     * Assignment operator.
     */
    KTp::LogEntity& operator=(const KTp::LogEntity &other);

    /**
     * Compare operator.
     */
    bool operator==(const KTp::LogEntity &other);

    /**
     * Compare operator.
     */
    bool operator!=(const KTp::LogEntity &other);

    /**
     * Returns whether this entity is valid (i.e. whether entity type is valid and
     * whether id is not empty).
     */
    bool isValid() const;

    /**
     * Returns ID of contact or chat room that this entity represents.
     */
    QString id() const;

    /**
     * Returns username of the contact or name of the chat room this entity represents
     * or an empty string if none was set.
     */
    QString alias() const;

    /**
     * Returns whether this entity represents a contact or a chat room.
     */
    Tp::HandleType entityType() const;

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

Q_DECLARE_METATYPE(KTp::LogEntity)

#endif // KTP_LOGENTITY_H
