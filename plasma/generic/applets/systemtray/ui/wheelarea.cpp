/***********************************************************************************************************************
 * System Tray (KDE Plasmoid)
 * Copyright ⓒ 2011-2012 ROSA  <support@rosalab.ru>
 * License: GPLv3+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **********************************************************************************************************************/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include <QtGui/QGraphicsSceneWheelEvent>

#include "wheelarea.h"

namespace SystemTray
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class WheelArea
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WheelArea::WheelArea(QDeclarativeItem *parent): QDeclarativeItem(parent)
{

}


void WheelArea::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!isEnabled())
        return;
    switch (event->orientation()) {
    case Qt::Vertical:
        emit scrollVert(event->delta());
        break;
    case Qt::Horizontal:
        emit scrollHorz(event->delta());
        break;
    default:
        event->ignore();
    }
}


} // namespace SystemTray
