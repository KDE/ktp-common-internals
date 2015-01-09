/*
    Copyright 2014  Nilesh Suthar <nileshsuthar@live.in>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef CHATDATASOURCE_H
#define CHATDATASOURCE_H

#include <kpeople/widgets/abstractfieldwidgetfactory.h>

namespace KTp
{
class PendingLoggerOperation;
}
class QStandardItemModel;

class ChatWidgetFactory : public KPeople::AbstractFieldWidgetFactory
{
    Q_OBJECT
public:
    explicit ChatWidgetFactory(QObject *parent, const QVariantList &args);
    virtual QString label() const;
    virtual int sortWeight() const;
    virtual QWidget* createDetailsWidget(const KPeople::PersonData &person, QWidget *parent) const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onPendingDates(KTp::PendingLoggerOperation *);
    void onEventsFinished(KTp::PendingLoggerOperation *);

private:
    QStandardItemModel *m_model;
};

#endif // CHATDATASOURCE_H
