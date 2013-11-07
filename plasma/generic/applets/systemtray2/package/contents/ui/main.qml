/***************************************************************************
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.private.systemtray 2.0 as SystemTray
import "plasmapackage:/code/Layout.js" as Layout

Item {
    id: root
    objectName: "SystemTrayRootItem"

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    property int minimumWidth: 200 // just needs to run out of space in the panel ...
    property int minimumHeight: 200 // ... but not too big to screw up initial layouts

    property int implicitWidth: baseSize * 25
    property int implicitHeight: baseSize * 20


    property int _h: itemSize // should go away, replace with root.baseSize

    // Sizes depend on the font, and thus on DPI
    property int baseSize: theme.mSize(theme.defaultFont).height
    property int itemSize: Layout.alignedSize(baseSize * 2)
    property int smallSpacing: Math.ceil(baseSize / 10)
    property int largeSpacing: Math.ceil(baseSize / 2)

    property bool debug: plasmoid.configuration.debug

    property Item expandedItem: null
    property string currentTask: ""

    property Component compactRepresentation: CompactRepresentation {
        systrayhost: host
    }


    Rectangle {
        anchors.fill: parent;
        border.width: 2;
        border.color: "black";
        color: "blue";
        visible: root.debug;
        opacity: 0.2;
    }
    SystemTray.Host {
        id: host
        rootItem: hiddenView
    }

    ExpandedRepresentation {
        anchors.fill: parent
    }
}