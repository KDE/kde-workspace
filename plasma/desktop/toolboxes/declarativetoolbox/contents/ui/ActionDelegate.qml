/***************************************************************************
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>                    *
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

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.qtextracomponents 0.1 as QtExtras

Item {
    id: toolBoxDelegate
    signal triggered
    property int iconSize: 22
    property Item view: unlockedList
    property string label: text.replace("&", "")
    property alias actionIcon: iconItem.icon
    height: toolBoxDelegate.iconSize + 14
    //width: parent.width
    width: 200


    Component.onCompleted: print("delegate text: " + label + objectName)

    QtExtras.QIconItem {
        id: iconItem
        height: toolBoxDelegate.iconSize
        width: toolBoxDelegate.iconSize
        anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 4 }
    }
    PlasmaComponents.Label {
        id: textLabel
        text:  (label != "") ? label : action.text.replace("&", "") // hack to get rid of keyboard accelerator hints
        elide: Text.ElideMiddle
        anchors { left: iconItem.right; right: parent.right; leftMargin: 6; verticalCenter: parent.verticalCenter; }
    }
    MouseArea {
        anchors.fill: parent
        anchors.margins: -6
        hoverEnabled: true
        onClicked: {
            if (typeof(index) == "undefined") {
                triggered();
            } else {
                trigger();
            }
        }
        onPressed: PlasmaExtras.PressedAnimation { targetItem: toolBoxDelegate }
        onReleased: PlasmaExtras.ReleasedAnimation { targetItem: toolBoxDelegate }
        onEntered: {
            toolBoxFrame.currentItem = toolBoxDelegate;
            toolBoxHighlight.opacity = 1;
            exitTimer.running = false;
        }
        onExited:  {
            if (toolBoxFrame.currentItem != null) {
                exitTimer.start()
            }
        }
    }
}
