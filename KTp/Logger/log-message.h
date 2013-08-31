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

#ifndef KTP_LOGMESSAGE_H
#define KTP_LOGMESSAGE_H

#include <KTp/message.h>
#include <KTp/Logger/log-entity.h>
#include <KTp/ktp-export.h>

namespace KTp {

/**
 * Represents a single log message.
 *
 * TODO: This class is useless, it's just a workaround for KTp::Message having
 *       a protected constructor and we have no other access to KTp::MessagePrivate
 *       We already had to tweak the KTp::Message anyway, so maybe tweaking it
 *       a bit more to avoid having to have this class would not matte that much
 *
 * @since 0.7
 * @author Daniel Vrátil <dvratil@redhat.com>
 */
class KTP_EXPORT LogMessage : public KTp::Message
{
  public:
    explicit LogMessage(const KTp::LogEntity &from, const Tp::AccountPtr &account,
                        const QDateTime &dt, const QString &message,
                        const QString &messageToken);
    LogMessage(const LogMessage& other);

    virtual ~LogMessage();
};

} // namespace KTp

#endif // KTP_LOGMESSAGE_H
