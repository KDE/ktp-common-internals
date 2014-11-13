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

#ifndef KTP_ABSTRACT_MESSAGE_FILTER_H
#define KTP_ABSTRACT_MESSAGE_FILTER_H

#include <KTp/message.h>
#include <KTp/outgoing-message.h>

#include <KTp/message-context.h>
#include <KTp/ktpcommoninternals_export.h>

#include <TelepathyQt/Account>
#include <TelepathyQt/TextChannel>

namespace KTp
{

class KTPCOMMONINTERNALS_EXPORT AbstractMessageFilter : public QObject
{
    Q_OBJECT

  public:
    AbstractMessageFilter(QObject* parent = 0);
    virtual ~AbstractMessageFilter();

    /** Filter messages to show on the UI recieved by another contact*/
    virtual void filterMessage(KTp::Message &message, const KTp::MessageContext &context);

    /** Scripts that must be included in the <head> section of the html required by this message filter.*/
    virtual QStringList requiredScripts();

    /** Stylesheets that must be included in the <head> section of the html required by this message filter.*/
    virtual QStringList requiredStylesheets();

    /** Filter composed messages about to be sent to telepathy backend */
    virtual void filterOutgoingMessage(KTp::OutgoingMessage &message, const KTp::MessageContext &context);
};

}

#endif // ABSTRACTPLUGIN_H

