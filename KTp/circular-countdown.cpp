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

#include "circular-countdown.h"

#include <QPainter>
#include <QPaintEvent>
#include <QTimeLine>

namespace KTp
{

class CircularCountdown::Private
{
public:
    Private(CircularCountdown *parent)
        : q(parent)
    {

    }

    CircularCountdown *q;
    QTimeLine *timeLine;
};

CircularCountdown::CircularCountdown(int msec, QWidget *parent)
    : QWidget(parent),
      d(new Private(this))
{
    setAutoFillBackground(false);

    d->timeLine = new QTimeLine(msec, this);
    //circle has 360 degrees, for better smoothness we use 2x as much
    d->timeLine->setFrameRange(0, 720);
    //to paint the subtraction animation, we start from full circle to 0
    d->timeLine->setDirection(QTimeLine::Backward);

    //repaint on every frame change for smooth animation
    connect(d->timeLine, SIGNAL(frameChanged(int)), this, SLOT(repaint()));

    //repaint after animation is finished
    connect(d->timeLine, SIGNAL(finished()), this, SLOT(repaint()));

    //emit timeoutReached() when the timeout is reached
    connect(d->timeLine, SIGNAL(finished()), this, SIGNAL(timeout()));
}

CircularCountdown::~CircularCountdown()
{
}

void CircularCountdown::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    if (d->timeLine->state() == QTimeLine::Running || d->timeLine->state() == QTimeLine::Paused) {
        QPainter painter(this);
        //always take parent widget's palette and use it's Base color
        painter.setBrush(QBrush(parentWidget()->palette().color(QPalette::Base), Qt::SolidPattern));
        painter.setRenderHint(QPainter::Antialiasing);
        /* drawPie always paints 1/16th of a degree, the total circle is 5760 (16 * 360)
         * the first argument is this widget size with 2px padding
         * second argument is start position, which is 3 o'clock by default,
         * to move it to 12 o'clock we need to start at 90 degrees, hence 90 * 16
         * third argument tells how much of the current circle is painted
         * the range is [0..720], hence the *8 (to get 5760 in total)
         * and it's minus because we want it to rotate in the other direction
         */
        painter.drawPie(this->rect().adjusted(2, 2, -2, -2), 90*16, -d->timeLine->currentFrame()*8);
    }
}

QSize CircularCountdown::sizeHint() const
{
    return QSize(16, 16);
}

void CircularCountdown::setDuration(int msec) {
    d->timeLine->setDuration(msec);
}

int CircularCountdown::duration() const
{
    return d->timeLine->duration();
}

void CircularCountdown::start() {
    d->timeLine->start();
}

void CircularCountdown::stop() {
    d->timeLine->stop();
}

void CircularCountdown::pause() {
    //no, there really is no ->pause() if you're thinking about that ;)
    d->timeLine->setPaused(true);
}

void CircularCountdown::resume() {
    d->timeLine->resume();
}

}
