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
#include "main-window.h"
#include "debug-message-view.h"
#include <KAction>
#include <KIcon>
#include <KStatusBar>
#include <KToolBar>


MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    setCentralWidget(new QWidget(this));
    m_ui.setupUi(centralWidget());
    setupGUI();

    m_ui.mcLogsView->setService(QLatin1String("org.freedesktop.Telepathy.MissionControl5"));
    m_ui.gabbleLogsView->setService(QLatin1String("org.freedesktop.Telepathy.ConnectionManager.gabble"));
    m_ui.hazeLogsView->setService(QLatin1String("org.freedesktop.Telepathy.ConnectionManager.haze"));
    m_ui.salutLogsView->setService(QLatin1String("org.freedesktop.Telepathy.ConnectionManager.salut"));
    m_ui.rakiaLogsView->setService(QLatin1String("org.freedesktop.Telepathy.ConnectionManager.rakia"));

    KAction *saveLogAction = new KAction(KIcon(QLatin1String("document-save-as"), KIconLoader::global()), i18n("Save Log"), this);
    saveLogAction->setToolTip(i18nc("Toolbar icon tooltip", "Save log of the current tab"));
    toolBar()->addAction(saveLogAction);

    connect(m_ui.mcLogsView, SIGNAL(statusMessage(QString)), statusBar(), SLOT(showMessage(QString)));
    connect(m_ui.gabbleLogsView, SIGNAL(statusMessage(QString)), statusBar(), SLOT(showMessage(QString)));
    connect(m_ui.hazeLogsView, SIGNAL(statusMessage(QString)), statusBar(), SLOT(showMessage(QString)));
    connect(m_ui.salutLogsView, SIGNAL(statusMessage(QString)), statusBar(), SLOT(showMessage(QString)));
    connect(m_ui.rakiaLogsView, SIGNAL(statusMessage(QString)), statusBar(), SLOT(showMessage(QString)));
    connect(saveLogAction, SIGNAL(triggered(bool)), this, SLOT(saveLogFile()));
}

MainWindow::~MainWindow()
{

}

void MainWindow::saveLogFile()
{
    m_ui.tabWidget->currentWidget()->findChild<DebugMessageView *>()->saveLogFile();
}