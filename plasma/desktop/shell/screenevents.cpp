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


#include "screenevents.h"

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

ScreenEvents::ScreenEvents()
    : QObject()
    , m_desktop(qApp->desktop())
{
    connect(m_desktop, SIGNAL(resized(int)), SLOT(resized(int)));
    connect(m_desktop, SIGNAL(screenCountChanged(int)), SLOT(screenCountChanged(int)));

    cacheScreenGeometry();
}

ScreenEvents::~ScreenEvents()
{

}

void ScreenEvents::cacheScreenGeometry()
{
    for (int id = 0; id < m_desktop->numScreens(); ++id) {
        m_screenGeometry.insert(id, m_desktop->screenGeometry(id));
    }
}

void ScreenEvents::resized(int id)
{
    Q_ASSERT(m_screenGeometry.contains(id));

    QRect oldRect = m_screenGeometry[id];
    QRect newRect = m_desktop->screenGeometry(id);

    if (oldRect.topLeft() != newRect.topLeft()) {
        Q_EMIT screenMoved(id, oldRect.topLeft(), newRect.topLeft());
    }

    if (oldRect.size() != newRect.size()) {
        Q_EMIT screenResized(id, oldRect.size(), newRect.size());
    }
}

void ScreenEvents::screenCountChanged(int numScreens)
{
    for(int id = 0; id < numScreens; ++id) {
        if (m_screenGeometry.contains(id)) {
            continue;
        }


    }
}