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
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: compactRepresentation

    PlasmaCore.IconItem {
        property int s: Math.min(parent.height, parent.width)
        anchors {
            horizontalCenter: parent.horizontalCenter
            //fill: parent
        }
        y: -height / 8 /// Urghs, but works due to the character being rather static
        width: s
        height: s

        source: root.state == "playing" ? "media-playback-pause" : "media-playback-start"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (expandedLoader.source == "") {
                expandedLoader.setSource("ExpandedRepresentation.qml", { 'anchors.fill': root});
            }
            plasmoid.expanded = !plasmoid.expanded
        }
       // onPressed: PlasmaExtras.PressedAnimation { targetItem: parent }
       // onReleased: PlasmaExtras.ReleasedAnimation { targetItem: parent }
    }
}