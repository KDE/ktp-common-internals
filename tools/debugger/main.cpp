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
#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>
#include <TelepathyQt/Debug>
#include <TelepathyQt/Types>

int main(int argc, char **argv)
{
    KAboutData aboutData("ktp-debugger", 0,
            ki18n("KDE Telepathy Debug Tool"),
            "0.4.60",
            ki18n("Tool for inspecting logs of the various underlying telepathy components"),
            KAboutData::License_LGPL,
            ki18n("Copyright (C) 2011 Collabora Ltd."));
    aboutData.addAuthor(ki18n("George Kiagiadakis"), ki18n("Developer"),
                        "george.kiagiadakis@collabora.com");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    Tp::registerTypes();
    Tp::enableDebug(true);
    Tp::enableWarnings(true);

    MainWindow *mw = new MainWindow;
    mw->show();

    return app.exec();
}
