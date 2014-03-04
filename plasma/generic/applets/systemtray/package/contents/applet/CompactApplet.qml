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
import QtQuick.Window 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaCore.ToolTipArea {
    id: root
    objectName: "org.kde.desktop-CompactApplet"
    anchors.fill: parent

    icon: plasmoid.icon
    mainText: plasmoid.toolTipMainText
    subText: plasmoid.toolTipSubText
    location: root.parent.parent.location
    property Item fullRepresentation
    property Item compactRepresentation
    property Item expandedFeedback: expandedItem

    PlasmaCore.FrameSvgItem {
        id: expandedItem
        anchors.fill: parent
        imagePath: "widgets/tasks"
        prefix: {
            var prefix;
            var location;

            if (plasmoid.parent.objectName == "taskListDelegate") {
                location = plasmoid.parent.location;
            } else {
                location = plasmoid.location;
            }

            switch (location) {
            case PlasmaCore.Types.LeftEdge:
                prefix = "west-focus";
                break;
            case PlasmaCore.Types.TopEdge:
                prefix = "north-focus";
                break;
            case PlasmaCore.Types.RightEdge:
                prefix = "east-focus";
                break;
            default:
                prefix = "south-focus";
            }

            if (!hasElementPrefix(prefix)) {
                prefix = "focus";
            }
            return prefix;
        }
        opacity: plasmoid.expanded ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: theme.shortDuration
                easing: Easing.InOutQuad
            }
        }
    }

    onCompactRepresentationChanged: {
        compactRepresentation.parent = root;
        compactRepresentation.anchors.fill = root;
        compactRepresentation.visible = true;
        root.visible = true;
    }
}
