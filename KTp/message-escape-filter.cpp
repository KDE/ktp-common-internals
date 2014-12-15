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

#include "message-filters-private.h"

#include <QTextDocument> //needed for Qt::escape

#include <KTp/text-parser.h>

MessageEscapeFilter::MessageEscapeFilter(QObject *parent)
    : KTp::AbstractMessageFilter(parent)
{
}

void MessageEscapeFilter::filterMessage(KTp::Message &message, const KTp::MessageContext &context)
{
    Q_UNUSED(context)

    // Here we do URL detection first, before any escaping is done.
    // If escaping would be done first, the link detection of eg. "<http://kde.org/>"
    // would become &lt;http://kde.org/&gt; and the URL filter would
    // match "http://kde.org/&gt" as the link. On the other hand if it's escaped afterwards,
    // it would also escape the newly inserted <a href...> links and the user would
    // get &lt;a href.../a&gt; and no clickable links.
    //
    // Therefore we first detect the links, replace them with placeholders,
    // then escape everything, then replace placeholders with actual links.

    QString messageText = message.mainMessagePart();

    QVariantList urls = message.property("Urls").toList();

    // link detection
    KTp::TextUrlData parsedUrl = KTp::TextParser::instance()->extractUrlData(messageText);

    QList<QPair<QString, QString> > placeholders;

    int offset = 0;
    for (int i = 0; i < parsedUrl.fixedUrls.size(); i++) {
         QUrl url(parsedUrl.fixedUrls.at(i));
         if (url.scheme() != QLatin1String("mailto")) {
             QString originalText = messageText.mid(parsedUrl.urlRanges.at(i).first + offset, parsedUrl.urlRanges.at(i).second);
             QString link = QString::fromLatin1("<a href=\"%1\">%2</a>").arg(QString::fromLatin1(url.toEncoded()), originalText);

             QString placeholder = QString::fromLatin1("#K#T#P%1").arg(i);

             // replace the link with a placeholder^ so it passes through the escaping phase,
             // then it will be replaced back for the actual link
             messageText.replace(parsedUrl.urlRanges.at(i).first + offset, parsedUrl.urlRanges.at(i).second, placeholder);

             placeholders << qMakePair(placeholder, link);

             urls.append(url);

             //after the first replacement is made, the original position values are not valid anymore, this adjusts them
             offset += placeholder.length() - originalText.length();
         }
     }

    message.setProperty("Urls", urls);

    QString escapedMessage = messageText.toHtmlEscaped();

    escapedMessage.replace(QLatin1String("\n "), QLatin1String("<br/>&nbsp;")); //keep leading whitespaces
    escapedMessage.replace(QLatin1Char('\n'), QLatin1String("<br/>"));
    escapedMessage.replace(QLatin1Char('\r'), QLatin1String("<br/>"));
    escapedMessage.replace(QLatin1Char('\t'), QLatin1String("&nbsp; &nbsp; ")); // replace tabs by 4 spaces
    escapedMessage.replace(QLatin1String("  "), QLatin1String(" &nbsp;")); // keep multiple whitespaces

    // replace link placeholders with actual links
    for (int i = 0; i < placeholders.size(); i++) {
        escapedMessage.replace(placeholders.at(i).first, placeholders.at(i).second);
    }

    message.setMainMessagePart(escapedMessage);
}
