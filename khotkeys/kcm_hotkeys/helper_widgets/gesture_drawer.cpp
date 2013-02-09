/****************************************************************************

 KHotKeys

 Copyright (C) 2003 Mike Pilone <mpilone@slac.com>
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>
 Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#include <QtGui/QColor>
#include <QtGui/QPainter>
#include <QtGui/QFrame>
#include <QtGui/QPaintEvent>
#include <QtGui/QGradient>

#include "gesture_drawer.h"

GestureDrawer::GestureDrawer(QWidget *parent, const char *name)
  : QFrame(parent), _data(KHotKeys::StrokePoints())
    {
    setObjectName(name);
    QPalette p;
    p.setColor( backgroundRole(), palette().color( QPalette::Base ) );
    setPalette( p );
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setMinimumSize(30, 30);
    }


GestureDrawer::~GestureDrawer()
    {
    }


KHotKeys::StrokePoints GestureDrawer::pointData() const
    {
    return _data;
    }


void GestureDrawer::setPointData(const KHotKeys::StrokePoints &data)
    {
    _data = data;

    repaint();
    }

void GestureDrawer::paintEvent(QPaintEvent *ev)
    {
    const int n = _data.size();

    if( n < 2 )
        {
        QFrame::paintEvent(ev);
        return;
        }

    const int border=6;

    const int l = width() < height() ? width() : height();
    const int x_offset = border + ( width()-l )/2;
    const int y_offset = border + ( height()-l )/2;


    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setWidth(4);
    pen.setCapStyle(Qt::RoundCap);

    // starting point
    double x = x_offset + _data[0].x * (l - 2*border);
    double y = y_offset + _data[0].y * (l - 2*border);

    for(int i=0; i<n-1; i++)
        {

        double nextx = x_offset + _data[i+1].x * (l - 2*border);
        double nexty = y_offset + _data[i+1].y * (l - 2*border);

        QLinearGradient grad( x, y, nextx, nexty );
        const int startRed   = 0;
        const int startGreen = ( 1-_data[i].s ) * 255;
        const int startBlue  = _data[i].s * 255;
        const int endRed     = 0;
        const int endGreen   = ( 1-_data[i+1].s ) * 255;
        const int endBlue    = _data[i+1].s * 255;
        QColor startColor( startRed, startGreen, startBlue );
        QColor endColor( endRed, endGreen, endBlue );
        grad.setColorAt( 0, startColor );
        grad.setColorAt( 1, endColor );

        pen.setBrush(grad);
        p.setPen(pen);
        p.drawLine(x, y, nextx, nexty);

        x=nextx;
        y=nexty;
        }


    QFrame::paintEvent(ev);
    }

#include "moc_gesture_drawer.cpp"
