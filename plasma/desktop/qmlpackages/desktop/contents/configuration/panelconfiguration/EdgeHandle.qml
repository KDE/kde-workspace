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
    text: i18n("Screen Edge")

    QtExtras.MouseEventListener {
        anchors.fill: parent
        property int lastX
        property int lastY
        property int startMouseX
        property int startMouseY
        onPressed: {
            lastX = mouse.screenX
            lastY = mouse.screenY
            startMouseX = mouse.x
            startMouseY = mouse.y
        }
        onPositionChanged: {
            panel.screen = mouse.screen;

            switch (panel.location) {
            case PlasmaCore.Types.TopEdge:
                configDialog.y = mouse.screenY - mapToItem(dialogRoot, 0, startMouseY).y
                panel.y = configDialog.y - panel.height
                break
            case PlasmaCore.Types.LeftEdge:
                configDialog.x = mouse.screenX - mapToItem(dialogRoot, startMouseX, 0).x
                panel.x = configDialog.x - panel.width
                break;
            case PlasmaCore.Types.RightEdge:
                configDialog.x = mouse.screenX - mapToItem(dialogRoot, startMouseX, 0).x
                panel.x = configDialog.x + configDialog.width
                break;
            case PlasmaCore.Types.BottomEdge:
            default:
                configDialog.y = mouse.screenY - mapToItem(dialogRoot, 0, startMouseY).y
                panel.y = configDialog.y + configDialog.height
            }

            lastX = mouse.screenX
            lastY = mouse.screenY

            //If the mouse is in an internal rectangle, do nothing
            if ((mouse.screenX > panel.screenGeometry.x + panel.screenGeometry.width/3 &&
                 mouse.screenX < panel.screenGeometry.x + panel.screenGeometry.width/3*2) &&
                (mouse.screenY > panel.screenGeometry.y + panel.screenGeometry.height/3 &&
                 mouse.screenY < panel.screenGeometry.y + panel.screenGeometry.height/3*2)) {
                return;
            }

            var screenAspect = panel.screenGeometry.height / panel.screenGeometry.width
            var newLocation = panel.location

            if (mouse.screenY < panel.screenGeometry.y+(mouse.screenX-panel.screenGeometry.x)*screenAspect) {
                if (mouse.screenY < panel.screenGeometry.y + panel.screenGeometry.height-(mouse.screenX-panel.screenGeometry.x)*screenAspect) {
                    newLocation = PlasmaCore.Types.TopEdge;
                } else {
                    newLocation = PlasmaCore.Types.RightEdge;
                }

            } else {
                if (mouse.screenY < panel.screenGeometry.y + panel.screenGeometry.height-(mouse.screenX-panel.screenGeometry.x)*screenAspect) {
                    newLocation = PlasmaCore.Types.LeftEdge;
                } else {
                    newLocation = PlasmaCore.Types.BottomEdge;
                }
            }

            if (panel.location != newLocation) {
                print("New Location: " + newLocation);
            }
            panel.location = newLocation
        }
        onReleased: {
            panelResetAnimation.running = true
        }
    }
}
