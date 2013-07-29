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
import org.kde.draganddrop 1.0

import "../code/layout.js" as Layout

PlasmaCore.Dialog {
    property Item target

    visible: false
    location: tasks.location
    windowFlags: Qt.Popup

    mainItem: Item {
        MouseHandler {
            anchors.fill: parent

            id: mouseHandler

            target: taskList
        }

        TaskList {
            id: taskList

            anchors.fill: parent

            Repeater {
                id: groupRepeater

                onCountChanged: updateSize()
            }
        }
    }

    onVisibleChanged: {
        if (visible && target) {
            groupFilter.model = tasksModel;
            groupFilter.rootIndex = visualModel.modelIndex(target.itemIndex);
            groupRepeater.model = groupFilter;
            updatePosition();
        } else {
            groupRepeater.model = undefined;
            groupFilter.model = undefined;
            groupFilter.rootIndex = undefined;
        }
    }

    onHeightChanged: updatePosition()

    function updatePosition() {
        if (target) {
            var pos = groupDialog.popupPosition(target, Qt.AlignCenter);
            x = pos.x;
            y = pos.y;
        }
    }

    function updateSize() {
        if (!visible || !target) {
            return;
        }

        if (!groupRepeater.count) {
            visible = false;
        } else {
            var task;
            var maxWidth = 0;

            for (var i = 0; i < taskList.children.length - 1; ++i) {
                task = taskList.children[i];

                if (task.textWidth > maxWidth) {
                    maxWidth = task.textWidth;
                }

                task.textWidthChanged.connect(updateSize);
            }

            maxWidth += Layout.horizontalMargins() + theme.smallIconSize + 6;

            // TODO: Properly derive limits from work area size (screen size sans struts).
            mainItem.width = Math.min(maxWidth, (tasks.vertical ? 640 - tasks.width : Math.max(tasks.width, 640)) - 20);
            mainItem.height = groupRepeater.count * (Layout.verticalMargins() + theme.smallIconSize);
        }
    }

    VisualDataModel {
        id: groupFilter
        delegate: Task {
            visible: true
            inPopup: true
        }
    }
}
