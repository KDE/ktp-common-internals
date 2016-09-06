/*
    Copyright (C) 2011 Collabora Ltd. <info@collabora.com>
      @author George Kiagiadakis <george.kiagiadakis@collabora.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DEBUG_MESSAGES_MODEL_H
#define DEBUG_MESSAGES_MODEL_H

#include <QWidget>
#include <TelepathyQt/Types>
#include <KTextEditor/Document>

class TelepathyProcess;

class DebugMessageView : public QWidget
{
    Q_OBJECT
public:
    explicit DebugMessageView(QWidget *parent = 0);
    ~DebugMessageView();

    void showEvent(QShowEvent *event);
    void setTelepathyProcess(TelepathyProcess *process);
    void saveLogFile();

private Q_SLOTS:
    void appendMessage(const Tp::DebugMessage &msg);
    void addDelayedMessages();
    void clear();

private:
    Tp::DebugMessageList m_tmpCache;
    KTextEditor::Document *m_editor;
};

#endif // DEBUG_MESSAGES_MODEL_H
