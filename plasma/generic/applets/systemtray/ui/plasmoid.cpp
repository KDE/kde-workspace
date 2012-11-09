/***********************************************************************************************************************
 * KDE System Tray (Plasmoid)
 *
 * Copyright (C) 2012 ROSA  <support@rosalab.ru>
 * License: GPLv2+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 **********************************************************************************************************************/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include "plasmoid.h"

#include "../core/task.h"

#include <inttypes.h>

#include <QtGui/QMenu>

#include <KDE/Plasma/IconWidget>
#include <KDE/KAction>
#include <KDE/Plasma/Containment>
#include <KDE/Plasma/Corona>
#include <KDE/KWindowSystem>


namespace SystemTray
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Plasmoid
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Plasmoid::Plasmoid(SystemTray::Applet *parent)
    : QObject(parent),
      m_applet(parent)
{
}


Plasmoid::~Plasmoid()
{
}





QPoint Plasmoid::popupPosition(QVariant item_var, QSize size, int align) const
{
    QGraphicsItem *item = qobject_cast<QGraphicsItem*>(item_var.value<QObject*>());
    if (m_applet) {
        if ( item && m_applet->containment() && m_applet->containment()->corona() ) {
            return m_applet->containment()->corona()->popupPosition(item, size, (Qt::AlignmentFlag)align);
        }
        return m_applet->popupPosition(size, (Qt::AlignmentFlag)align);
    }
    return QPoint();
}




} // namespace SystemTray
