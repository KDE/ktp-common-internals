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
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ui_main-window.h"

#include <TelepathyQt/Types>
#include <TelepathyQt/PendingOperation>

#include <KXmlGuiWindow>

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

public Q_SLOTS:
    void saveLogFile();

private Q_SLOTS:
    void onAccountManagerBecameReady(Tp::PendingOperation *pendingReady);

private:
    void initConnectionManagerTabs(const QSet<QString> &connectionManagerSet);

    Tp::AccountManagerPtr m_AccountManager;
    Ui::MainWindow m_ui;
};

#endif // MAIN_WINDOW_H
