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
}

MainWindow::~MainWindow()
{

}

