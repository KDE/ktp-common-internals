/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef KTP_PROXY_PENDING_CURRY_OPERATION_HEADER
#define KTP_PROXY_PENDING_CURRY_OPERATION_HEADER

#include <TelepathyQt/PendingOperation>

#include <QObject>


class Extractor
{
    public:
        virtual ~Extractor();
        virtual void operator()(Tp::PendingOperation *op) = 0;
};

class PendingCurryOperation : public Tp::PendingOperation
{
    Q_OBJECT

    public:
        PendingCurryOperation(Tp::PendingOperation *op, Extractor *ex, const Tp::SharedPtr<Tp::RefCounted> &obj);
        ~PendingCurryOperation();

        Extractor& extractor();

    private Q_SLOTS:
        void onFinished(Tp::PendingOperation *op);

    private:
        Extractor *ex;

};

#endif
