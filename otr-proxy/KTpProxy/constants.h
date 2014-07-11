/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef KTP_PROXY_CONSTANTS_HEADER
#define KTP_PROXY_CONSTANTS_HEADER

#define KTP_PROXY_BUS_NAME (QString::fromLatin1("org.freedesktop.Telepathy.Client.KTp.Proxy"))
#define KTP_PROXY_SERVICE_OBJECT_PATH (QString::fromLatin1("/org/freedesktop/TelepathyProxy/ProxyService"))
#define KTP_PROXY_CHANNEL_OBJECT_PATH_PREFIX (QString::fromLatin1("/org/freedesktop/TelepathyProxy/OtrChannelProxy/"))

#define KTP_PROXY_ERROR_NOT_CONNECTED (QString::fromLatin1("org.freedesktop.TelepathyProxy.Error.NotConnected"))

#endif
