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
import org.kde.qtextracomponents 2.0 as QtExtraComponents

import org.kde.private.systemtray 2.0 as SystemTray
import "plasmapackage:/code/Layout.js" as Layout


QtExtraComponents.MouseEventListener {
    id: taskItemContainer
    objectName: "taskItemContainer"

    width: root.itemSize
    height: root.itemSize + (root.smallSpacing * 2)

    hoverEnabled: true

    // opacity is raised when: plasmoid is collapsed, we are the current task, or it's hovered
    opacity: (containsMouse || !plasmoid.expanded || root.currentTask == taskId) || (plasmoid.expanded && root.currentTask == "") ? 1.0 : 0.6
    Behavior on opacity { NumberAnimation { duration: 150 } }

    property int taskStatus: status
    property int taskType: type
    property Item expandedItem: taskItemExpanded
    property Item expandedStatusItem: null
    property alias icon: itemIcon

    Rectangle {
        anchors.fill: parent;
        border.width: 1;
        border.color: "violet";
        color: "pink";
        visible: root.debug;
        opacity: 0.5;
    }

    Component {
        id: exandedStatusItemComponent
        Item {
            Rectangle {
                anchors.fill: parent;
                border.width: 1;
                border.color: "black";
                color: "green";
                visible: root.debug;
                opacity: 0.7;
            }

        }
    }

    onExpandedItemChanged: {
        if (expandedItem && root.expandedItem != expandedItem) {
            root.currentTask = taskId;
            root.expandedItem = expandedItem;
        } else if (root.currentTask == taskId) {
            // release
            root.currentTask = ""
            root.expandedItem = null;
        }
    }

    PlasmaCore.IconItem {
        id: itemIcon
        width: parent.height
        height: parent.height
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
        source: iconName != "" ? iconName : (typeof(icon) != "undefined" ? icon : "")
    }

    PulseAnimation {
        targetItem: taskItemContainer
        running: status == SystemTray.Task.NeedsAttention
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            print("ST2B TaskDelegate clicked ... " + !taskItemContainer.expanded + taskItemContainer.expandedItem);
            if (root.currentTask == taskId) {
                root.currentTask = 0;
                root.expandedItem = null;
                plasmoid.expanded = false;
            } else if (taskItemContainer.expanded || taskItemContainer.taskStatus == SystemTray.Task.TypeStatusItem) {
                print("Setting taskitem");
                root.currentTask = taskId;
                root.expandedItem = taskItemContainer.expandedItem;
            } else {
                print("resetting taskitem");
                root.currentTask = "";
                root.expandedItem = null;
            }
        }
    }
    onWidthChanged: updatePlasmoidGeometry()
    onHeightChanged: updatePlasmoidGeometry()

    function updatePlasmoidGeometry() {
        if (taskItem != undefined) {

            var _size = root.itemSize;
            var _m = (taskItemContainer.height - _size) / 2

            taskItem.anchors.verticalCenter = taskItemContainer.verticalCenter;
            taskItem.x = _m;
            taskItem.height = _size;
            taskItem.width = _size;
        }
    }

    Component.onCompleted: {
        if (taskType == SystemTray.Task.TypeStatusItem) {
            var component = Qt.createComponent("ExpandedStatusNotifier.qml");
            if (component.status == Component.Ready) {
                print("ST2P Loading STatusItemExpanded: ");
                expandedItem = component.createObject(taskItemContainer, {"x": 300, "y": 300});
            } else {
                print("Error loading statusitem: " + component.errorString());
            }
        } else if (taskItem != undefined) {
            taskItem.parent = taskItemContainer;
            updatePlasmoidGeometry();
        }
    }
}
