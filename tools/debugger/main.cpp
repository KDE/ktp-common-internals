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

#include "ktp_version.h"

#include <KAboutData>
#include <KLocalizedString>
#include <TelepathyQt/Debug>
#include <TelepathyQt/Types>

int main(int argc, char **argv)
{
    KAboutData aboutData(QStringLiteral("ktp-debugger"),
            i18n("KDE Telepathy Debug Tool"),
            QStringLiteral(KTP_VERSION_STRING),
            i18n("Tool for inspecting logs of the various underlying telepathy components"),
            KAboutLicense::LGPL,
            i18n("Copyright (C) 2011 Collabora Ltd."));
    aboutData.addAuthor(i18n("George Kiagiadakis"), i18n("Developer"),
                        QStringLiteral("george.kiagiadakis@collabora.com"));

    QApplication app(argc, argv);
    {
        QCommandLineParser parser;
        aboutData.setupCommandLine(&parser);
        parser.process(app);
        aboutData.processCommandLine(&parser);
    }

    Tp::registerTypes();
    Tp::enableDebug(true);
    Tp::enableWarnings(true);

    MainWindow *mw = new MainWindow;
    mw->show();

    return app.exec();
}
