/*
* Copyright (C) 2011 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TELEPATHY_HANDLER_APPLICATION_H
#define TELEPATHY_HANDLER_APPLICATION_H

#include <QApplication>

#include <KTp/ktpcommoninternals_export.h>

namespace KTp
{

/**
 * \brief A KApplication that exits the application when there are no running jobs
 *
 * Morover it does automatically another few things required by every handler:
 * - It automatically register Telepathy-Qt Types
 * - setQuitOnLastWindowClosed(false)
 * - Adds the --persist option to inhibit automatic exit.
 * - Adds the --debug option to enable telepathy-qt4 debug
 * - Enables telepathy-qt4 warnings
 * - Use Tp-Qt4 callback for redirecting debug output into KDebug
 * - Sets the KDE_FULL_SESSION environment variable to workaround dbus activation issues.
 */
class KTPCOMMONINTERNALS_EXPORT TelepathyHandlerApplication : public QApplication
{
    Q_OBJECT

public:
    /**
     * \p initialTimeout Initial timeout time (in msec) after which application exits if no jobs are received.
     * \p timeout Timeout time (in msec) after which application exits after the last job is finished.
     */
    explicit TelepathyHandlerApplication(int &argc, char *argv[],
                                         int initialTimeout = 15000,
                                         int timeout = 2000);

    virtual ~TelepathyHandlerApplication();

    static int newJob();
    static void jobFinished();

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void _k_onInitialTimeout())
    Q_PRIVATE_SLOT(d, void _k_onTimeout())
};

} // namespace KTp

#endif // TELEPATHY_HANDLER_APPLICATION_H
