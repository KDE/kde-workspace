/*
 *   Copyright 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef SCREENEVENTS_H
#define SCREENEVENTS_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QRect>

class QDesktopWidget;

class ScreenEvents : public QObject
{
    Q_OBJECT
    public:
        ScreenEvents();
        virtual ~ScreenEvents();

    private Q_SLOTS:
        void resized(int id);
        void screenCountChanged(int  numScreens);

    private:
        void cacheScreenGeometry();

        QDesktopWidget *m_desktop;
        QHash<int, QRect>  m_screenGeometry;
};

#endif // SCREENEVENTS_H
