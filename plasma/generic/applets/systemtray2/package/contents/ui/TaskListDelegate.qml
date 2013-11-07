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

import org.kde.private.systemtray 2.0 as SystemTray


TaskDelegate {
    id: taskListDelegate
    objectName: "taskListDelegate"

    property bool expanded: (root.currentTask == "")

    width: parent.width
    height: root.itemSize + root.largeSpacing
    //height: itemSize

//     onExpandedItemChanged: {
//         print("ST2P TaskDelegate Expanded changed ...");
//         if (expandedItem != undefined) {
// //             expandedItem.anchors.fill = expandedItemContainer;
//         }
//     }

    MouseArea {
        anchors.fill: parent

    }

//     Column {
//         id: labels
//         width: parent.width
//         height: mainLabel.height * 2
//         anchors {
//             //fill: parent
//         }
    PlasmaComponents.Label {
        id: mainLabel
        //x: taskListDelegate.height + root.largeSpacing
        anchors {
            left: icon.right
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        visible: taskListDelegate.expanded
        text: name
        elide: Text.ElideRight
    }
//         PlasmaComponents.Label {
//             width: parent.width
//             font.pointSize: theme.defaultFont.pointSize - 2
//             opacity: 0.7
//             text: tooltipTitle + "<br />" + tooltipText
//             elide: Text.ElideRight
//             clip: true
//         }
//    }
}
