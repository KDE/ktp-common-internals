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

#include "text-parser.h"

#include <QtCore/QLatin1String>

namespace KTp
{

TextParser* TextParser::s_instance = NULL;

/**
 * RegExp for url detection
 */
static QRegExp s_urlPattern(QString::fromLatin1("\\b((?:(?:([a-z][\\w\\.-]+:/{1,3})|www\\d{0,3}[.]|[a-z0-9.\\-]+[.][a-z]{2,4}/)(?:[^\\s()<>]+|\\(([^\\s()<>]+|(\\([^\\s()<>]+\\)))*\\))+(?:\\(([^\\s()<>]+|(\\([^\\s()<>]+\\)))*\\)|\\}\\]|[^\\s`!()\\[\\]{};:'\".,<>?%1%2%3%4%5%6])|[a-z0-9.\\-+_]+@[a-z0-9.\\-]+[.][a-z]{1,5}[^\\s/`!()\\[\\]{};:'\".,<>?%1%2%3%4%5%6]))")
                            .arg(QChar(0x00AB)).arg(QChar(0x00BB)).arg(QChar(0x201C)).arg(QChar(0x201D)).arg(QChar(0x2018)).arg(QChar(0x2019)));

TextParser::TextParser(QObject* parent)
    : QObject(parent)
{
}

TextParser* TextParser::instance()
{
    if (!s_instance) {
        s_instance = new TextParser(0);
    }

    return s_instance;
}

TextUrlData TextParser::extractUrlData(const QString& text, bool doUrlFixup)
{
    TextUrlData data;
    QString htmlText(text);
    s_urlPattern.setCaseSensitivity(Qt::CaseInsensitive);

    int pos = 0;
    int urlLen = 0;

    QString protocol;
    QString href;

    while ((pos = s_urlPattern.indexIn(htmlText, pos)) >= 0) {
        urlLen = s_urlPattern.matchedLength();
        href = htmlText.mid(pos, urlLen);

        data.urlRanges << QPair<int, int>(pos, href.length());
        pos += href.length();

        if (doUrlFixup) {
            protocol.clear();
            if (s_urlPattern.cap(2).isEmpty()) {
                QString urlPatternCap1(s_urlPattern.cap(1));
                if (urlPatternCap1.contains(QLatin1Char('@'))) {
                    protocol = QLatin1String("mailto:");
                } else if (urlPatternCap1.startsWith(QLatin1String("ftp."), Qt::CaseInsensitive)) {
                    protocol = QLatin1String("ftp://");
                } else {
                    protocol = QLatin1String("http://");
                }
            }

            href = protocol + href;
            data.fixedUrls.append(href);
        }
    }
    return data;
}

TextParser::~TextParser()
{
}

}
