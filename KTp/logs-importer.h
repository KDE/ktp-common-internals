/*
 * Copyright (C) 2012 Dan Vrátil <dvratil@redhat.com>
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

#ifndef LOGSIMPORTER_H
#define LOGSIMPORTER_H

#include <QtCore/QObject>
#include <TelepathyQt/Types>

#include <KTp/ktp-export.h>

namespace KTp
{

/**
 * @short A class to import old logs from Kopete
 *
 * This class provides convenient interface for importing logs from Kopete.
 *
 * Currently the importer supports AIM, WML, ICQ, Jabber, GaduGadu and Yahoo logs.
 */
class KTP_EXPORT LogsImporter : public QObject
{
  Q_OBJECT

  public:
    LogsImporter(QObject *parent = 0);

    virtual ~LogsImporter();

    /**
     * Checks whether there are any Kopete logs for \p account.
     *
     * @param account Telepathy Account against whose Kopete counterpart to check
     * @return Returns when there is at least one log for given account
     */
    bool hasKopeteLogs(const Tp::AccountPtr &account);

    /**
     * Imports Kopete logs for \p account to Telepathy
     *
     * Finds all Kopete logs for Kopete-version of \p account, converts them
     * to Telepathy Logger format and imports then to Telepathy Logger storage.
     *
     * This method returns immediatelly. When all logs are scanned and converted,
     * logsImported() signal is emitted.
     *
     * The import will NOT overwrite existing log files.
     *
     * @param account A Telepathy Account
     */
    void startLogImport(const Tp::AccountPtr &account);

  Q_SIGNALS:
    /**
     * Emitted when logs are successfully imported.
     */
    void logsImported();

    /**
     * Emitted when an error occurs during importing.
     *
     * The process can still import some logs, but some might be missing.
     */
    void error(const QString &error);

  private:
    class Private;
    Private *d;
};

} /* namespace KTp */

#endif // LOGSIMPORTER_H
