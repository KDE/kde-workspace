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

Item {

    anchors.margins: theme.largeSpacing

    function checkTask(task) {
        if (task.taskItemExpanded == null) return;
        var isthis = (task.taskId == root.currentTask);
        if (!isthis) {
            task.expanded = false;
        } else {
            root.currentName = task.name;
        }
    }

    function clearExpanded() {
        var _hidden = host.hiddenTasks;
        for (var i = 0; i < _hidden.length; i++) {
            checkTask(_hidden[i]);
        }
        var _shown = host.shownTasks;
        for (i = 0; i < _shown.length; i++) {
            checkTask(_shown[i]);
        }
    }

    Connections {
        target: root
        onExpandedItemChanged: {
            if (root.expandedItem != null) {
                print("not null");

                root.expandedItem.parent = expandedItemContainer;
                root.expandedItem.anchors.fill = expandedItemContainer;
                expandedItemContainer.replace(root.expandedItem);
            } else {
                if (expandedItemContainer.currentPage != null) {
                    expandedItemContainer.clear();
                }
                root.currentTask = "";
            }
            clearExpanded();
        }
    }
    /* This mechanism hides inactive items until the layout figures out that it has to
     * collapse the Plasmoid.
     *
     * It also takes care of delaying the loading of hidden items (they're hidden, and
     * can be loaded later as to not block other, visible components from loading). It
     * does so by simply pushing them out of the viewport, which means the ListView won't
     * render the TaskDelegates until then.
     */
    /*
    Item {
        id: loadingItem
        anchors.fill: parent
        anchors.margins: height
        Timer {
            running: true
            interval: 4000
            onTriggered: loadingItem.visible = false
        }
    }
    */
    MouseArea {
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: expandedItemContainer.left
        }
        onClicked: {
            clearExpanded();
            expandedItemContainer.clear();
            root.currentTask = ""
            root.currentName = ""
            root.expandedItem = null;
        }
    }

    ListView {
        id: hiddenView
        objectName: "hiddenView"
        clip: true
        width: parent.width

        interactive: (contentHeight > height)

        anchors {
            top: snHeading.bottom
            topMargin: theme.largeSpacing / 2
            bottom: parent.bottom
            left: parent.left
        }
        spacing: 4

        model: host.hiddenTasks

        delegate: TaskListDelegate {}
    }

    PlasmaCore.SvgItem {
        id: separator

        width: lineSvg.elementSize("vertical-line").width;
        height: parent.width;
        visible: root.expandedItem != null

        anchors {
            right: expandedItemContainer.left;
            rightMargin: theme.largeSpacing
            //top: parent.right;
            bottom: parent.bottom;
            //rightMargin: theme.largeSpacing
        }
        elementId: "vertical-line";

        svg: PlasmaCore.Svg {
            id: lineSvg;
            imagePath: "widgets/line";
        }
    }

    Connections {
        target: plasmoid
        onExpandedChanged: {
            if (!plasmoid.expanded) {
                root.expandedItem = null;
                expandedItemContainer.clear();
                root.currentTask = "";
            }
            clearExpanded();
        }
    }

    PlasmaExtras.Heading {
        id: snHeading

        level: 1
        opacity: root.currentTask != "" ? 0 : 0.8
        Behavior on opacity { NumberAnimation {} }

        anchors {
            //margins: theme.largeSpacing
            top: parent.top
            leftMargin: -theme.largeSpacing
            left: expandedItemContainer.left
            right: parent.right
        }
        text: i18n("Status & Notifications")
    }

    PlasmaExtras.Heading {
        id: snHeadingExpanded

        level: 1
        opacity: root.currentTask != "" ? 0.6 : 0
        Behavior on opacity { NumberAnimation {} }

        anchors {
            //margins: theme.largeSpacing
            top: parent.top
            //leftMargin: root.currentTask != "" ? 0 : -theme.largeSpacing
            left: expandedItemContainer.left
            right: parent.right
        }
        text: root.currentName
    }

    PlasmaComponents.PageStack {
        id: expandedItemContainer
        anchors {
            left: parent.left
            leftMargin: (root.baseSize + theme.largeSpacing * 3)
            top: snHeading.bottom
            topMargin: theme.largeSpacing / 2
            bottom: parent.bottom
            right: parent.right
        }
    }
    MouseArea {
        id: pin

        /* Allows the user to keep the calendar open for reference */

        width: theme.largeSpacing
        height: width
        anchors {
            top: parent.top
            right: parent.right
            rightMargin: -theme.largeSpacing
            topMargin: -theme.largeSpacing
        }

        property bool checked: false

        onClicked: {
            pin.checked = !pin.checked;
            plasmoid.hideOnWindowDeactivate = !pin.checked;
        }

        PlasmaComponents.Label {
            width: paintedWidth
            height: paintedHeight
            anchors.centerIn: parent
            text: "✓"
            opacity: pin.checked ? 1 : 0.3
        }
    }

    Component.onCompleted: {
        clearExpanded();
    }
}