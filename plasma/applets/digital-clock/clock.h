/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick                          *
 *   siraj@kdemail.net                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef CLOCK_H
#define CLOCK_H

#include <QTime>
#include <QDate>
#include <QX11Info>

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/dialog.h>
#include "ui_clockConfig.h"
#include "ui_calendar.h"
#include <kdatepicker.h>


class KDialog;

namespace Plasma
{
    class Svg;
}

class Clock : public Plasma::Applet
{
    Q_OBJECT
    public:
        Clock(QObject *parent, const QVariantList &args);
        ~Clock();

        void init();
        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
        void setPath(const QString&);
        void constraintsUpdated(Plasma::Constraints);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);

        // reimplemented
        Qt::Orientations expandingDirections() const;

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void showConfigurationInterface();

    protected slots:
        void configAccepted();
        void showCalendar(QGraphicsSceneMouseEvent *event);

    private:
        Q_ENUMS( m_clockStyle )
        enum ClockStyle {
            PlainClock, FancyClock
        };

        int m_clockStyle;
        QFont m_plainClockFont;
        QColor m_plainClockColor;
        bool m_plainClockFontBold;
        bool m_plainClockFontItalic;

        bool m_showDate;
        bool m_showYear;
        bool m_showDay;
        bool m_showTimezone;
        bool m_calendarIsShown;

        QSize m_defaultElementSize;

        QSizeF m_sizeHint;
        int m_pixelSize;
        QString m_timezone;
        Plasma::Svg* m_theme;
        QTime m_time;
        QDate m_date;
        KDialog *m_dialog; //should we move this into another class?
        Plasma::Dialog *m_calendar;
        QVBoxLayout *m_layout;
        QTime m_lastTimeSeen;
        /// Designer Config file
        Ui::clockConfig ui;
        Ui::calendar m_calendarUi;

        bool m_animating;
        int m_animationStep;

        // Default spacings
        int m_horizontalSpacing;
        int m_verticalSpacing;
};

K_EXPORT_PLASMA_APPLET(dig_clock, Clock)

#endif
