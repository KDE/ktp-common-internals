/*
    Copyright (C) 2016  Martin Klapetek <mklapetek@kde.org>

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

#include "text-area-emojis-handler.h"

#include <QQuickTextDocument>
#include <QQuickItem>
#include <QVariant>
#include <QRegularExpressionMatchIterator>
#include <QTextDocumentFragment>
#include <QTextCursor>
#include <QDebug>

#include <KEmoticons>

TextAreaEmojisHandler::TextAreaEmojisHandler(QObject *parent)
    : QObject(parent),
      m_emojiMatcher(QStringLiteral("(^|\\s)(:[^\\s]+:)")),
      m_emojiImgMatcher(QStringLiteral("<img.+src=\"([^\\s]+)\".+/>"))
{
    KEmoticons *kemoticons = new KEmoticons();
    kemoticons->setPreferredEmoticonSize(QSize(32, 32));
    m_theme = kemoticons->theme(QStringLiteral("EmojiOne"));
}

TextAreaEmojisHandler::~TextAreaEmojisHandler()
{

}

void TextAreaEmojisHandler::setTextArea(QQuickItem *textArea)
{
    m_document = 0;
    m_textArea = textArea;

    if (!m_textArea) {
        return;
    }

    QVariant doc = m_textArea->property("textDocument");
    if (doc.canConvert<QQuickTextDocument*>()) {
        QQuickTextDocument *qqdoc = doc.value<QQuickTextDocument*>();
        if (qqdoc) {
            m_document = qqdoc->textDocument();
        }
    }

    Q_EMIT textAreaChanged();

    connect(m_document, &QTextDocument::contentsChanged, this, &TextAreaEmojisHandler::onTextChanged);
}

QQuickItem* TextAreaEmojisHandler::textArea() const
{
    return m_textArea;
}

void TextAreaEmojisHandler::onTextChanged()
{
    QRegularExpressionMatchIterator matches = m_emojiMatcher.globalMatch(m_document->toPlainText());
    if (!matches.hasNext()) {
        return;
    }

    QString text = m_document->toPlainText();
    QTextCursor cursor = QTextCursor(m_document);

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();

        QString emojiHtml;
        if (match.capturedStart() != 0) {
            emojiHtml = QStringLiteral(" ");
        }

        emojiHtml.append(m_theme.parseEmoticons(match.captured(2)))
                 .append(QStringLiteral(" "));

        cursor.setPosition(match.capturedStart(2));
        cursor.setPosition(match.capturedEnd(2), QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.insertHtml(emojiHtml);
    }
}

QString TextAreaEmojisHandler::getText() const
{
    auto documentHtml = m_document->toHtml();
    auto emojiImgs = m_emojiImgMatcher.globalMatch(documentHtml);

    while (emojiImgs.hasNext()) {
        auto emoji = emojiImgs.next();
        documentHtml.replace(emoji.capturedStart(0), emoji.capturedLength(0), asciiEmojiForPath(emoji.captured(1)));
    }

    QTextDocument *tempDocument = new QTextDocument();
    tempDocument->setHtml(documentHtml);

    int start = qBound(0, 0, tempDocument->characterCount() - 1);
    int end = qBound(0, 100, tempDocument->characterCount() - 1);
    QTextCursor cursor(tempDocument);
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    return cursor.selection().toPlainText().trimmed();
}

QString TextAreaEmojisHandler::asciiEmojiForPath(const QString &filePath) const
{
    const auto emojiValues = m_theme.emoticonsMap().value(filePath);
    Q_FOREACH (const QString &emoji, emojiValues) {
        if (emoji.startsWith(QLatin1Char(':'))) {
            return emoji;
        }
    }
    return emojiValues.at(0);
}
