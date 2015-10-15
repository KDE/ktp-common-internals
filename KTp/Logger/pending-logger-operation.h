/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
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
 *
 */

#ifndef KTP_PENDINGLOGGEROPERATION_H
#define KTP_PENDINGLOGGEROPERATION_H

#include <QtCore/QObject>
#include <KTp/ktpcommoninternals_export.h>

namespace KTp {

class AbstractLoggerPlugin;
class LogManager;

class KTPCOMMONINTERNALS_EXPORT PendingLoggerOperation : public QObject
{
    Q_OBJECT

  public:
    virtual ~PendingLoggerOperation();

    bool hasError() const;
    QString error() const;

  Q_SIGNALS:
    void finished(KTp::PendingLoggerOperation *self);

  protected:
    explicit PendingLoggerOperation(QObject *parent = 0);

    void setError(const QString &error);
    void emitFinished();

    QList<KTp::AbstractLoggerPlugin*> plugins() const;

  private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void __k__doEmitFinished());
};
}

#endif // KTP_PENDINGLOGGEROPERATION_H
