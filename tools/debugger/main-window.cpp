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

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/PendingReady>

#include <QtAlgorithms>

#include <QAction>
#include <QStatusBar>
#include <KToolBar>
#include <KLocalizedString>

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    setCentralWidget(new QWidget(this));
    m_ui.setupUi(centralWidget());
    setupGUI();

    m_ui.mcLogsView->setService(QLatin1String("org.freedesktop.Telepathy.MissionControl5"));

    m_AccountManager = Tp::AccountManager::create();
    Tp::PendingReady *pendingReady = m_AccountManager->becomeReady();
    connect(pendingReady, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onAccountManagerBecameReady(Tp::PendingOperation*)));

    QAction *saveLogAction = new QAction(QIcon::fromTheme(QLatin1String("document-save-as")), i18n("Save Log"), this);
    saveLogAction->setToolTip(i18nc("Toolbar icon tooltip", "Save log of the current tab"));
    toolBar()->addAction(saveLogAction);

    connect(m_ui.mcLogsView, SIGNAL(statusMessage(QString)), statusBar(), SLOT(showMessage(QString)));
    connect(saveLogAction, SIGNAL(triggered(bool)), this, SLOT(saveLogFile()));
}

MainWindow::~MainWindow()
{

}

void MainWindow::saveLogFile()
{
    m_ui.tabWidget->currentWidget()->findChild<DebugMessageView *>()->saveLogFile();
}

void MainWindow::onAccountManagerBecameReady(Tp::PendingOperation* op)
{
    if (op->isError()) {
        qCritical() << "Failed to initialize Tp::AccountManager"
                 << "Error was:" << op->errorName() << "-" << op->errorMessage();
    } else {
        QSet<QString> connectionManagers;

        Tp::AccountSetPtr accountSetPtr = m_AccountManager->onlineAccounts();
        QList<Tp::AccountPtr> accountList = accountSetPtr->accounts();

        Q_FOREACH(Tp::AccountPtr account, accountList) {
            connectionManagers.insert(account->cmName());
        }

        initConnectionManagerTabs(connectionManagers);
    }
}

void MainWindow::initConnectionManagerTabs(const QSet<QString>& connectionManagerSet)
{
    QStringList connectionManagerStringList = connectionManagerSet.toList();
    qSort(connectionManagerStringList);

    Q_FOREACH(QString connectionManager, connectionManagerStringList) {
        QWidget *cmTab = new QWidget();
        QHBoxLayout *horizontalLayout = new QHBoxLayout(cmTab);
        DebugMessageView *cmDebugMessageView = new DebugMessageView(cmTab);
        cmDebugMessageView->setService(QString(QLatin1String("org.freedesktop.Telepathy.ConnectionManager.%1")).arg(connectionManager));

        horizontalLayout->addWidget(cmDebugMessageView);

        // Convert the connectionManager to title case. eg. haze to Haze
        QString tabText = connectionManager;
        tabText[0] = tabText[0].toTitleCase();
        m_ui.tabWidget->addTab(cmTab, tabText);

        connect(cmDebugMessageView, SIGNAL(statusMessage(QString)), statusBar(), SLOT(showMessage(QString)));
    }
}
