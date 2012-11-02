// -*- coding: iso-8859-1 -*-
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtras

// import "plasmapackage:/code/LayoutManager.js" as LayoutManager

Item {
    id: main
    width: 540
    height: 540

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

    PlasmaCore.Svg {
        id: iconsSvg
        imagePath: "widgets/configuration-icons"
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
            interval: 500
            repeat: false
            running: true
            onTriggered: placeToolBox();
        }
    }

    ToolBoxItem {
        id: toolBoxItem
        anchors { top: parent.top; right: parent.right; }
    }

    function placeToolBox() {
        return;
        var ts = plasmoid.readConfig("ToolBoxButtonState")
        if (ts) {
            print("Read state from config: " + ts);
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
        print("Setting toolbox to: " + tx + "x" + ty + " screen: " + main.width+ "x" + main.height+"");
        toolBoxButton.x = tx;
        toolBoxButton.y = ty;

    }
}
