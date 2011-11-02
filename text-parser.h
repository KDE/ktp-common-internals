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

/**
 * Usefull data container
 *
 * @var QList urlRanges
 * @var QStringList fixedUrls
 */
struct TextUrlData
{
    QList<QPair<int, int> > urlRanges;
    QStringList fixedUrls;
};

/**
 * TextParser
 *
 */
class TextParser : public QObject
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
    TextParser(QObject *parent);

    /**
     * @var TextParser
     */
    static TextParser *s_instance;

    /**
     * @var QRegExp
     */
    static QRegExp s_urlPattern(QString("\\b((?:(?:([a-z][\\w\\.-]+:/{1,3})|www\\d{0,3}[.]|[a-z0-9.\\-]+[.][a-z]{2,4}/)(?:[^\\s()<>]+|\\(([^\\s()<>]+|(\\([^\\s()<>]+\\)))*\\))+(?:\\(([^\\s()<>]+|(\\([^\\s()<>]+\\)))*\\)|\\}\\]|[^\\s`!()\\[\\]{};:'\".,<>?%1%2%3%4%5%6])|[a-z0-9.\\-+_]+@[a-z0-9.\\-]+[.][a-z]{1,5}[^\\s/`!()\\[\\]{};:'\".,<>?%1%2%3%4%5%6]))").arg(QChar(0x00AB)).arg(QChar(0x00BB)).arg(QChar(0x201C)).arg(QChar(0x201D)).arg(QChar(0x2018)).arg(QChar(0x2019)));
};

#endif // TEXT_PARSER_H
