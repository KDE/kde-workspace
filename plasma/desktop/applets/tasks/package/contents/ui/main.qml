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

import QtQuick 1.1

import org.kde.plasma.core 0.1 as PlasmaCore

import Tasks 0.1 as Tasks

import "../code/layout.js" as Layout
import "../code/tools.js" as TaskTools

Item {
    id: tasks

    anchors.fill: parent

    property int location: 0
    property bool vertical: false
    property bool horizontal: false
    property int maxStripes: 2
    property bool forceStripes: false
    property bool showOnlyCurrentDesktop: false
    property bool showOnlyCurrentActivity: false
    property bool showOnlyMinimized: false
    property bool showToolTip: true
    property bool highlightWindows: false
    property bool manualSorting: false

    property int activeWindowId: 0

    property int optimumCapacity: Layout.optimumCapacity(width, height)

    property int preferredWidth: taskList.width
    property int preferredHeight: taskList.height

    property int minimumWidth: tasks.vertical ? 0 : Layout.preferredMinWidth()
    property int minimumHeight: tasks.horizontal ? 0 : Layout.preferredMinHeight()

    property Item dragSource: null

    signal activateItem(int id, bool toggle)
    signal itemContextMenu(int id)
    signal itemHovered(int id, bool hovered)
    signal itemMove(int id, int newIndex)
    signal itemGeometryChanged(int id, int x, int y, int width, int height)
    signal itemNeedsAttention(bool needs)
    signal presentWindows(int groupParentId)

    onWidthChanged: {
        taskList.width = Layout.layoutWidth();

        if (tasks.forceStripes) {
            taskList.height = Layout.layoutHeight();
        }
    }

    onHeightChanged: {
        if (tasks.forceStripes) {
            taskList.width = Layout.layoutWidth();
        }

        taskList.height = Layout.layoutHeight();
    }

    onActiveWindowIdChanged: {
        if (activeWindowId != groupDialog.windowId) {
            groupDialog.visible = false;
        }
    }

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

    Tasks.ToolTip {
        id: toolTip

        highlightWindows: tasks.highlightWindows

        function generateSubText(task) {
            var subTextEntries = new Array();

            if (!tasks.showOnlyCurrentDesktop) {
                subTextEntries.push(i18n("On %1", task.DesktopName));
            }

            if (task.OnAllActivities) {
                subTextEntries.push(i18nc("Which virtual desktop a window is currently on",
                    "Available on all activities"));
            } else if (tasks.showOnlyCurrentActivity) {
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

        model: tasksModel
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
                if (tasks.forceStripes) {
                    taskList.width = Layout.layoutWidth();
                    taskList.height = Layout.layoutHeight();
                }

                Layout.layout(taskRepeater);
            }
        }
    }

    GroupDialog { id: groupDialog }

    function resetDragSource() {
        dragSource = null;
    }

    function isTaskAt(pos) {
        var mapped = mapToItem(taskList, pos.x, pos.y);

        return (taskList.childAt(mapped.x, mapped.y) != null);
    }

    Component.onCompleted: {
        dragHelper.dropped.connect(resetDragSource);
    }
}
