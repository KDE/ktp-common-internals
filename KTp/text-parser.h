/*
 * Text Parser common class
 * Copyright (C) 2004 Peter Simonsson <psn@linux.se>
 * Copyright (C) 2006-2008 Eike Hein <hein@kde.org>
 * Copyright (C) 2011 Przemek Czekaj <xcojack@gmail.com>
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

#ifndef TEXT_PARSER_H
#define TEXT_PARSER_H

#include <QObject>
#include <QPair>
#include <QStringList>

#include <KTp/ktpcommoninternals_export.h>

namespace KTp
{

/**
 * Useful data container
 *
 * @var QList urlRanges
 * @var QStringList fixedUrls
 */
struct KTPCOMMONINTERNALS_EXPORT TextUrlData
{
    QList<QPair<int, int> > urlRanges;
    QStringList fixedUrls;
};

/**
 * TextParser
 *
 */
class KTPCOMMONINTERNALS_EXPORT TextParser : public QObject
{

public:
    /**
     * Singleton pattern
     *
     * @param void
     * @return TextParser
     */
    static TextParser *instance();

    /**
     * Method extract url from text
     *
     * @param QString string A whole text
     * @param bool doUrlFixup fix the url default true
     * @return TextUrlData
     * @author Konversation developers
     */
    TextUrlData extractUrlData(const QString& string, bool doUrlFixup = true);

    /**
     * Destructor
     *
     * @param void
     */
    virtual ~TextParser();

private:
    /**
     * Constructor
     *
     * @param QObject
     */
    TextParser(QObject *parent = 0);

    /**
     * @var TextParser
     */
    static TextParser *s_instance;
};

}

#endif // TEXT_PARSER_H
