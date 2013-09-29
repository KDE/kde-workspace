/***************************************************************************
 *   Copyright (C) 2012-2013 by Eike Hein <hein@kde.org>                   *
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

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.private.taskmanager 0.1 as TaskManager

import "../code/layout.js" as Layout
import "../code/tools.js" as TaskTools

Item {
    id: tasks

    anchors.fill: parent

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    property bool fillWidth: true
    property bool fillHeight:true
    property int minimumWidth: tasks.vertical ? 0 : Layout.preferredMinWidth()
    property int minimumHeight: !tasks.vertical ? 0 : Layout.preferredMinHeight()
    property int preferredWidth: taskList.width
    property int preferredHeight: taskList.height

    property Item dragSource: null

    signal activateItem(int id, bool toggle)
    signal itemContextMenu(Item item, QtObject configAction)
    signal itemHovered(int id, bool hovered)
    signal itemMove(int id, int newIndex)
    signal itemGeometryChanged(Item item, int id)

    onWidthChanged: {
        taskList.width = Layout.layoutWidth();

        if (plasmoid.configuration.forceStripes) {
            taskList.height = Layout.layoutHeight();
        }
    }

    onHeightChanged: {
        if (plasmoid.configuration.forceStripes) {
            taskList.width = Layout.layoutWidth();
        }

        taskList.height = Layout.layoutHeight();
    }

    TaskManager.Backend {
        id: backend

        taskManagerItem: tasks
        highlightWindows: plasmoid.configuration.highlightWindows

        groupingStrategy: plasmoid.configuration.groupingStrategy
        sortingStrategy: plasmoid.configuration.sortingStrategy

        onActiveWindowIdChanged: {
            if (activeWindowId != groupDialog.windowId) {
                groupDialog.visible = false;
            }
        }
    }

    Binding {
        target: backend.groupManager
        property: "screen"
        value: plasmoid.screen
    }

    Binding {
        target: backend.groupManager
        property: "onlyGroupWhenFull"
        value: plasmoid.configuration.onlyGroupWhenFull
    }

    Binding {
        target: backend.groupManager
        property: "fullLimit"
        value: Layout.optimumCapacity(width, height) + 1
    }

    Binding {
        target: backend.groupManager
        property: "showOnlyCurrentScreen"
        value: plasmoid.configuration.showOnlyCurrentScreen
    }

    Binding {
        target: backend.groupManager
        property: "showOnlyCurrentDesktop"
        value: plasmoid.configuration.showOnlyCurrentDesktop
    }

    Binding {
        target: backend.groupManager
        property: "showOnlyCurrentActivity"
        value: plasmoid.configuration.showOnlyCurrentActivity
    }

    Binding {
        target: backend.groupManager
        property: "showOnlyMinimized"
        value: plasmoid.configuration.showOnlyMinimized
    }

    TaskManager.DragHelper { id: dragHelper }

    PlasmaCore.FrameSvgItem {
        id: taskFrame

        visible: false;

        imagePath: "widgets/tasks";
        prefix: "normal"
    }

    PlasmaCore.Svg {
        id: arrows

        imagePath: "widgets/arrows"
        size: "16x16"
    }

    PlasmaCore.ToolTip {
        id: toolTip

        //FIXME TODO: highlightWindows: plasmoid.configuration.highlightWindows

        function generateSubText(task) {
            var subTextEntries = new Array();

            if (!plasmoid.configuration.showOnlyCurrentDesktop) {
                subTextEntries.push(i18n("On %1", task.DesktopName));
            }

            if (task.OnAllActivities) {
                subTextEntries.push(i18nc("Which virtual desktop a window is currently on",
                    "Available on all activities"));
            } else if (plasmoid.configuration.showOnlyCurrentActivity) {
                if (task.OtherActivityNames.length > 0) {
                    subTextEntries.push(i18nc("Activities a window is currently on (apart from the current one)",
                                              "Also available on %1",
                                              task.OtherActivityNames.join(", ")));
                }
            } else if (task.ActivityNames.length > 0) {
                subTextEntries.push(i18nc("Which activities a window is currently on",
                                          "Available on %1",
                                           task.ActivityNames.join(", ")));
            }

            return subTextEntries.join("<br />");
        }
    }

    MouseHandler {
        id: mouseHandler

        anchors.fill: parent

        target: taskList
    }

    VisualDataModel {
        id: visualModel

        model: backend.tasksModel
        delegate: Task {}
    }

    TaskList {
        id: taskList

        anchors {
            left: parent.left
            top: parent.top
        }

        onWidthChanged: Layout.layout(taskRepeater)
        onHeightChanged: Layout.layout(taskRepeater)

        flow: tasks.vertical ? Flow.TopToBottom : Flow.LeftToRight

        onAnimatingChanged: {
            if (!animating) {
                TaskTools.publishIconGeometries(children);
            }
        }

        Repeater {
            id: taskRepeater

            model: visualModel

            onCountChanged: {
                taskList.width = Layout.layoutWidth();
                taskList.height = Layout.layoutHeight();
                Layout.layout(taskRepeater);
            }
        }
    }

    GroupDialog { id: groupDialog }

    function updateStatus(demandsAttention) {
        if (demandsAttention) {
            plasmoid.status = PlasmaCore.Types.NeedsAttentionStatus;
        } else if (!backend.anyTaskNeedsAttention) {
            plasmoid.status = PlasmaCore.Types.PassiveStatus;
        }
    }

    function resetDragSource() {
        dragSource = null;
    }

    Component.onCompleted: {
        tasks.activateItem.connect(backend.activateItem);
        tasks.itemContextMenu.connect(backend.itemContextMenu);
        tasks.itemHovered.connect(backend.itemHovered);
        tasks.itemMove.connect(backend.itemMove);
        tasks.itemGeometryChanged.connect(backend.itemGeometryChanged);
        dragHelper.dropped.connect(resetDragSource);
    }
}
