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
import org.kde.qtextracomponents 2.0 as QtExtraComponents

import org.kde.private.systemtray 2.0 as SystemTray
import "plasmapackage:/code/Layout.js" as Layout


QtExtraComponents.MouseEventListener {
    id: taskItemContainer
    objectName: "taskItemContainer"

    height: root.itemSize + (units.smallSpacing * 2)
    width: snExpanded ? parent.width : height

    hoverEnabled: true

    property variant task: null
    property bool isCurrentTask: (root.currentTask == taskId)

    property bool isHiddenItem: false

    onTaskChanged: {
        //print("******************* Task changed:" + task.taskId + " " + task.name)
    }


    onClicked: {
        if (taskType == SystemTray.Task.TypePlasmoid) {
            togglePopup();
        }
    }

    Timer {
        id: hidePopupTimer
        interval: 10
        running: false
        repeat: false
        onTriggered: {
            print("hidetimer triggered, collapsing " + (root.currentTask == "") )
            if (root.currentTask == "") {
                plasmoid.expanded = false
            }
        }
    }

    // opacity is raised when: plasmoid is collapsed, we are the current task, or it's hovered
    opacity: (containsMouse || !plasmoid.expanded || root.currentTask == taskId) || (plasmoid.expanded && root.currentTask == "") ? 1.0 : 0.6
    Behavior on opacity { NumberAnimation { duration: 150 } }

    property int taskStatus: status
    property int taskType: type
    property Item expandedItem: taskItemExpanded
    property Item expandedStatusItem: null
    property bool snExpanded: false

    Rectangle {
        anchors.fill: parent;
        border.width: 1;
        border.color: "black";
        color: "yellow";
        visible: root.debug;
        opacity: 0.8;
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


    property bool isExpanded: expanded
    onIsExpandedChanged: {
        if (!expandedItem) {
            return;
        }
        print("AAA"+expanded+root.expandedItem+expandedItem)
        expandedItem.visible = false;
        if (expanded) {
            root.currentTask = taskId;
            root.expandedItem = expandedItem;
        } else {
            // release
            root.currentTask = ""
            root.expandedItem = null;
        }
    }
    onExpandedItemChanged: {
        if (!expandedItem) {
            return;
        }

        /*if (expanded && root.expandedItem == expandedItem) {
            root.expandedItem.visible = false;
            root.currentTask = taskId;
            root.expandedItem = expandedItem;
            expandedItem.visible = true;
        } else {
            expandedItem.visible = false;
        }*/
    }

    PulseAnimation {
        targetItem: taskItemContainer
        running: status == SystemTray.Task.NeedsAttention
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

    Loader {
        id: sniLoader
        anchors.fill: parent
    }

    Component.onCompleted: {
        if (taskType == SystemTray.Task.TypeStatusItem) {
            sniLoader.source = "StatusNotifierItem.qml";
            var component = Qt.createComponent("ExpandedStatusNotifier.qml");
            if (component.status == Component.Ready) {
                expandedItem = component.createObject(taskItemContainer, {"x": 300, "y": 300});
            } else {
                print("Error loading statusitem: " + component.errorString());
            }
        } else if (taskItem != undefined) {
            sniLoader.source = "PlasmoidItem.qml";
            taskItem.parent = taskItemContainer;
            taskItem.z = 999;
            updatePlasmoidGeometry();
        }
    }
}
