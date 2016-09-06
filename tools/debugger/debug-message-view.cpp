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
#include "debug-message-view.h"

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Constants>
#include <TelepathyQt/DebugReceiver>
#include <TelepathyQt/PendingDebugMessageList>

#include <QDebug>
#include <QAction>
#include <QFileDialog>
#include <KStandardAction>
#include <KLocalizedString>
#include <KService>
#include <KTextEditor/View>
#include <KTextEditor/Document>

#include <ctime>
#include <QDate>
#include <QPointer>
#include <QLayout>
#include <QMenu>

#include "telepathy-process.h"

DebugMessageView::DebugMessageView(QWidget *parent)
    : QWidget(parent)
{
    KService::Ptr service = KService::serviceByDesktopPath(QString::fromLatin1("katepart.desktop"));

    if (service) {
        m_editor = qobject_cast<KTextEditor::Document*>(service->createInstance<KParts::ReadWritePart>(this));
        Q_ASSERT(m_editor && "Failed to instantiate a KatePart");
    } else {
        qCritical() << "Could not find kate part";
    }

    m_editor->setReadWrite(false);
    KTextEditor::View* view = m_editor->createView(this);
    setLayout(new QHBoxLayout());
    layout()->addWidget(view);
    m_editor->setHighlightingMode(QString::fromLatin1("KTp"));

    view->setContextMenu(new QMenu());
    view->contextMenu()->addAction(KStandardAction::clear(this, SLOT(clear()), this));
}

DebugMessageView::~DebugMessageView()
{
}

void DebugMessageView::clear()
{
    m_editor->setReadWrite(true);
    m_editor->clear();
    m_editor->setReadWrite(false);
}

void DebugMessageView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    addDelayedMessages();
}

void DebugMessageView::setTelepathyProcess(TelepathyProcess *process)
{
    connect(process, SIGNAL(newDebugMessage(Tp::DebugMessage)),
            SLOT(appendMessage(Tp::DebugMessage)));
}

void DebugMessageView::addDelayedMessages()
{
    KTextEditor::Document::EditingTransaction transaction(m_editor);
    Q_FOREACH(const Tp::DebugMessage &msg, m_tmpCache) {
        appendMessage(msg);
    }
    m_tmpCache.clear();
}

//taken from empathy
static inline QString formatTimestamp(double timestamp)
{
    struct tm *tstruct;
    char time_str[32];
    int ms;
    time_t sec;

    ms = (int) ((timestamp - (int) timestamp)*1e6);
    sec = (long) timestamp;
    tstruct = std::localtime((time_t *) &sec);
    if (!std::strftime(time_str, sizeof(time_str), "%x %T", tstruct)) {
        qDebug() << "Failed to format timestamp" << timestamp;
        time_str[0] = '\0';
    }

    QString str;
    str.sprintf("%s.%06d", time_str, ms);
    return str;
}

void DebugMessageView::appendMessage(const Tp::DebugMessage &msg)
{
    if (isVisible()) {
        QString message = QString(formatTimestamp(msg.timestamp) %
                                QLatin1Literal(" - [") % msg.domain % QLatin1Literal("] ") %
                                msg.message);
        m_editor->setReadWrite(true);
        m_editor->insertText(m_editor->documentEnd(), message + QString::fromLatin1("\n"));
        m_editor->setReadWrite(false);
    } else {
        m_tmpCache.append(msg);
    }
}

void DebugMessageView::saveLogFile()
{
    QUrl savedFile = QUrl::fromUserInput(QFileDialog::getSaveFileName(this, i18nc("@title:window", "Save Log")));
    m_editor->saveAs(savedFile);
}
