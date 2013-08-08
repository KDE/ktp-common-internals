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
 * TODO: This is not nice; we are just workarounding the fact, that KTp::Message
 *       has a protected constructor and we have no other access to KTp::MessagePrivate
 */
class KTP_EXPORT LogMessage : public KTp::Message
{
  public:
    explicit LogMessage(const KTp::LogEntity &from, const Tp::AccountPtr &account,
                        const QDateTime &dt, const QString &message);
    virtual ~LogMessage();
};

}

#endif // KTP_LOGMESSAGE_H
