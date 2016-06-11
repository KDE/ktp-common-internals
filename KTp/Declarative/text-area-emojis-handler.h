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

#ifndef TEXTAREAEMOJISHANDLER_H
#define TEXTAREAEMOJISHANDLER_H

#include <QObject>
#include <QRegularExpression>
#include <KEmoticonsTheme>

class QTextDocument;
class QQuickItem;

class TextAreaEmojisHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *textArea READ textArea WRITE setTextArea NOTIFY textAreaChanged)

public:
    TextAreaEmojisHandler(QObject *parent = 0);
    ~TextAreaEmojisHandler();

    void setTextArea(QQuickItem *textArea);
    QQuickItem *textArea() const;

    Q_INVOKABLE QString getText() const;

Q_SIGNALS:
    void textAreaChanged();
    void shouldShowAutocompletion(const QString &text);

private Q_SLOTS:
    void onTextChanged();

private:
    QString asciiEmojiForPath(const QString &filePath) const;

    QTextDocument *m_document;
    QQuickItem *m_textArea;
    QRegularExpression m_emojiMatcher;
    QRegularExpression m_emojiImgMatcher;
    KEmoticonsTheme m_theme;
};

#endif
