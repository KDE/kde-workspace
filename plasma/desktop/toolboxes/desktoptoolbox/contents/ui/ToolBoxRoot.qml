/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.qtextracomponents 2.0 as QtExtras

Item {
    id: main

    anchors.fill: parent

    signal minimumWidthChanged
    signal minimumHeightChanged
    signal maximumWidthChanged
    signal maximumHeightChanged
    signal preferredWidthChanged
    signal preferredHeightChanged

    property int iconSize: 16
    property variant availScreenRect: plasmoid.availableScreenRegion(plasmoid.screen)[0]
    property int iconWidth: 22
    property int iconHeight: iconWidth

    MouseArea {
        id: toolBoxDismisser
        anchors.fill: parent
        visible: toolBoxItem.showing
        onPressed: toolBoxItem.showing = false
    }

    PlasmaCore.Svg {
        id: toolBoxSvg
        imagePath: "widgets/toolbox"
        property int rightBorder: elementSize("right").width
        property int topBorder: elementSize("top").height
        property int bottomBorder: elementSize("bottom").height
        property int leftBorder: elementSize("left").width
    }

    ToolBoxButton {
        id: toolBoxButton
        Timer {
            id: placeToolBoxTimer
            interval: 100
            repeat: false
            running: true
            onTriggered: {
                print("XXX PLacing toolbox");
                placeToolBox();
            }
        }
    }

    ToolBoxItem {
        id: toolBoxItem
        //anchors { top: parent.top; right: parent.right; margins: 16; }
        property int margin: 22
        x: {
            var maxX = main.width - toolBoxItem.width - margin
            if (toolBoxButton.x > maxX) {
                return maxX;
            } else {
                return Math.max(toolBoxButton.x, margin);
            }
        }
        y: {
            var maxY = main.height - toolBoxItem.height - margin
            if (toolBoxButton.y > maxY) {
                return maxY;
            } else {
                return Math.max(toolBoxButton.y, margin);
            }
        }
        anchors.margins: 16
    }

    function placeToolBox() {
        //return;
        print(" XXXXXXXXXXXX  placeToolBox");
        var ts = plasmoid.readConfig("ToolBoxButtonState")
        ts = "topright"; // FIXME: hardcoded for now, test config saving!
        if (ts) {
            //print("Read state from config: " + ts);
            if (ts == "topleft") {
                toolBoxButton.x = 0;
                toolBoxButton.y = 0;
                return;
            } else if (ts == "topright") {
                toolBoxButton.x = main.width - toolBoxButton.width;
                toolBoxButton.y = 0;
                return;
            } else if (ts == "bottomright") {
                toolBoxButton.x = main.width - toolBoxButton.width;
                toolBoxButton.y = main.height - toolBoxButton.height;
                return;
            } else if (ts == "bottomleft") {
                toolBoxButton.x = 0;
                toolBoxButton.y = main.height - toolBoxButton.height;
                return;
            }
        }

        var tx = plasmoid.readConfig("ToolBoxButtonX")
        if (tx) tx = main.width - toolBoxButton.width;

        var ty = plasmoid.readConfig("ToolBoxButtonY")
        if (ty) ty = 0;
        print("XXX Setting toolbox to: " + tx + "x" + ty + " screen: " + main.width+ "x" + main.height+"");
        toolBoxButton.x = tx;
        toolBoxButton.y = ty;

    }

    Component.onCompleted: {
        print(" XXXX TOolboxroot done");
    }

    function i18n(msg) {
        print("FIXME: i18n() is screwed in ToolBoxRoot.qml")
        return msg;

    }
}
