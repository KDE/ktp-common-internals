/*
    Circular countdown widget
    Copyright (C) 2011  Martin Klapetek <martin.klapetek@gmail.com>

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

#ifndef CIRCULARCOUNTDOWN_H
#define CIRCULARCOUNTDOWN_H

#include <QWidget>

#include <KTp/ktpcommoninternals_export.h>

namespace KTp
{

class KTPCOMMONINTERNALS_EXPORT CircularCountdown : public QWidget
{
    Q_OBJECT

public:
    explicit CircularCountdown(int msec = 5000, QWidget *parent = 0);
    ~CircularCountdown();

    void setDuration(int msec);
    int duration() const;

public Q_SLOTS:
    void start();
    void stop();
    void pause();
    void resume();

Q_SIGNALS:
    void timeout();


protected:
     void paintEvent(QPaintEvent *event);
     QSize sizeHint() const;

private:
    class Private;
    Private * const d;
};

}

#endif // CIRCULARCOUNTDOWN_H
