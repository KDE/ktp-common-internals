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

#ifndef KTP_LOGSIMPORTER_PRIVATE_H
#define KTP_LOGSIMPORTER_PRIVATE_H

#include <QThread>
#include <QStringList>
#include <QDomDocument>

#include "logs-importer.h"

namespace KTp {

class LogsImporter::Private: public QThread
{
    Q_OBJECT

  public:
    Private(LogsImporter* parent);
    ~Private();

    void setAccountId(const QString &accountId);
    QStringList findKopeteLogs(const QString &accountId) const;

  Q_SIGNALS:
    void error(const QString &error);

  protected:
    virtual void run();

  private:

    QString accountIdToProtocol(const QString &accountId) const;
    QString accountIdToAccountName(const QString &accountId) const;

    void initKTpDocument();
    void saveKTpDocument();
    QDateTime parseKopeteTime(const QDomElement &kopeteMessage) const;
    QDomElement convertKopeteMessage(const QDomElement &kopeteMessage);
    void convertKopeteLog(const QString &filepath);

    QString m_accountId;
    QString m_meId;
    QString m_contactId;

    QDomDocument m_ktpDocument;
    QDomDocument m_kopeteDocument;
    QDomElement m_ktpLogElement;

    int m_day;
    int m_month;
    int m_year;

    bool m_isMUCLog;

    friend class KTp::LogsImporter;
};

} /* namespace KTp */

#endif // KTP_LOGSIMPORTER_PRIVATE_H
