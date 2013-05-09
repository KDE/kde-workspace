/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0
import org.kde.qtextracomponents 2.0 as QtExtras

PlasmaComponents.ToolButton {
    flat: false
    text: "Height"
    QtExtras.MouseEventListener {
        anchors.fill: parent
        property int startMouseX
        property int startMouseY
        onPressed: {
            startMouseX = mouse.x
            startMouseY = mouse.y
        }
        onPositionChanged: {
            switch (panel.location) {
            //TopEdge
            case 3:
                configDialog.y = mouse.screenY - mapToItem(dialogRoot, 0, startMouseY).y
                panel.thickness = configDialog.y - panel.y
                break;
            //LeftEdge
            case 5:
                configDialog.x = mouse.screenX - mapToItem(dialogRoot, startMouseX, 0).x
                panel.thickness = configDialog.x - panel.x
                break;
            //RightEdge
            case 6:
                configDialog.x = mouse.screenX - mapToItem(dialogRoot, startMouseX, 0).x
                panel.thickness = (panel.x + panel.width) - (configDialog.x + configDialog.width)
                break;
            //BottomEdge
            case 4:
            default:
                configDialog.y = mouse.screenY - mapToItem(dialogRoot, 0, startMouseY).y
                panel.thickness = (panel.y + panel.height) - (configDialog.y + configDialog.height)
            }
        }
    }
}
