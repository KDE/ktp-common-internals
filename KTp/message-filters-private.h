/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>

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

#ifndef MESSAGE_FILTERS_PRIVATE_H
#define MESSAGE_FILTERS_PRIVATE_H

#include "abstract-message-filter.h"

#include <QObject>

class MessageUrlFilter : public KTp::AbstractMessageFilter
{
  public:
    explicit MessageUrlFilter(QObject *parent = 0);
    void filterMessage(KTp::Message &message, const KTp::MessageContext &context);
};

class MessageEscapeFilter : public KTp::AbstractMessageFilter
{
  public:
    explicit MessageEscapeFilter(QObject *parent = 0);
    virtual void filterMessage(KTp::Message& message, const KTp::MessageContext &context);
};

#endif
