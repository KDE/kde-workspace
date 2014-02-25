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
import org.kde.draganddrop 2.0

import "../code/layout.js" as LayoutManager

PlasmaCore.Dialog {
    visible: false

    type: PlasmaCore.Dialog.PopupMenu
    flags: Qt.WindowStaysOnTopHint
    hideOnWindowDeactivate: true
    location: plasmoid.location

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

    data: [
        VisualDataModel {
            id: groupFilter

            delegate: Task {
                visible: true
                inPopup: true
            }
        }
    ]

    onVisualParentChanged: {
        if (!visualParent) {
            visible = false;
        }
    }

    onVisibleChanged: {
        if (visible && visualParent) {
            groupFilter.model = backend.tasksModel;
            groupFilter.rootIndex = groupFilter.modelIndex(visualParent.itemIndex);
            groupRepeater.model = groupFilter;
        } else {
            visualParent = null;
            groupRepeater.model = undefined;
            groupFilter.model = undefined;
            groupFilter.rootIndex = undefined;
        }
    }

    function updateSize() {
        if (!visible || !visualParent) {
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

            maxWidth += LayoutManager.horizontalMargins() + units.iconSizes.small + 6;

            // TODO: Properly derive limits from work area size (screen size sans struts).
            mainItem.width = Math.min(maxWidth, (tasks.vertical ? 640 - tasks.width : Math.max(tasks.width, 640)) - 20);
            mainItem.height = groupRepeater.count * (LayoutManager.verticalMargins() + units.iconSizes.small);
        }
    }
}
