/*
 * Copyright (C) 2014 Martin Klapetek <mklapetek@kde.org>
 * Copyright (C) 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
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
 */

#ifndef KTP_MODELS_DEBUG_H
#define KTP_MODELS_DEBUG_H

#include <QLoggingCategory>
// include the QDebug here so there doesn't have to
// be two debug includes in the files using qCDebug
#include <QDebug>
Q_DECLARE_LOGGING_CATEGORY(KTP_MODELS)

#endif
