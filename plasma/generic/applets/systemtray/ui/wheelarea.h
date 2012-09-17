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


#ifndef __SYSTEMTRAY__WHEELAREA_H
#define __SYSTEMTRAY__WHEELAREA_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Includes
#include <QtDeclarative/QDeclarativeItem>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
class QGraphicsSceneWheelEvent;


namespace SystemTray
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class WheelArea
/** @class WheelArea
 * This helper class is introduced to provide mouse wheel functionality prior to Qt 5.
 */
class WheelArea: public QDeclarativeItem
{
    Q_OBJECT
public:
    explicit WheelArea(QDeclarativeItem *parent = 0);

private: //Events
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

signals:
    // Signal is emmitted as user scrolls up or down
    void scrollVert(int delta);

    // Signal is emmitted as user scrolls left or right
    void scrollHorz(int delta);
};

} // namespace SystemTray

#endif // __SYSTEMTRAY__WHEELAREA_H