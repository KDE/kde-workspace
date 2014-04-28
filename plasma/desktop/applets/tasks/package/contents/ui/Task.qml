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
import org.kde.qtextracomponents 0.1

import Tasks 0.1 as Tasks

import "../code/layout.js" as Layout
import "../code/tools.js" as TaskTools

MouseEventListener {
    id: task

    width: groupDialog.mainItem.width
    height: theme.smallIconSize + Layout.verticalMargins()

    visible: false

    LayoutMirroring.enabled: (Qt.application.layoutDirection == Qt.RightToLeft)
    LayoutMirroring.childrenInherit: (Qt.application.layoutDirection == Qt.RightToLeft)

    property int itemIndex: index
    property int itemId: model.Id
    property bool inPopup: false
    property bool isGroupParent: model.hasModelChildren
    property bool isLauncher: model.IsLauncher
    property bool isStartup: model.IsStartup
    property bool demandsAttention: model.DemandsAttention
    property int textWidth: label.implicitWidth
    property int pressX: -1
    property int pressY: -1
    property Item busyIndicator

    hoverEnabled: true

    onItemIndexChanged: {
        if (!inPopup && !tasks.vertical && Layout.calculateStripes() > 1) {
            var newWidth = Layout.taskWidth();

            if (index == tasksModel.launcherCount) {
                newWidth += Layout.launcherLayoutWidthDiff();
            }

            width = newWidth;
        }
    }

    onIsStartupChanged: {
        if (!isStartup) {
            tasks.itemGeometryChanged(itemId, x, y, width, height);
        }
    }

    onDemandsAttentionChanged: {
        tasks.itemNeedsAttention(demandsAttention);
    }

    onContainsMouseChanged:  {
        if (containsMouse) {
            if (!inPopup && tasks.showToolTip) {
                toolTip.target = frame;
                toolTip.mainText = model.DisplayRole;
                toolTip.image = model.DecorationRole;
                toolTip.subText = model.IsLauncher ? model.GenericName
                    : toolTip.generateSubText(model);
                toolTip.windowsToPreview = model.WindowList;
            } else {
                // A bit sneaky, but this works well to hide the tooltip.
                toolTip.target = taskFrame;
            }
        } else {
            pressX = -1;
            pressY = -1;
        }

        tasks.itemHovered(model.Id, containsMouse);
    }

    onClicked: {
        if (isGroupParent) {
            if (mouse.modifiers & Qt.ControlModifier) {
                tasks.presentWindows(model.Id);
            } else {
                groupDialog.target = task;
                groupDialog.visible = true;
            }
        } else {
            tasks.activateItem(model.Id, true);
        }
    }

    onPressed: {
        if (mouse.buttons & Qt.LeftButton) {
            pressX = mouse.x;
            pressY = mouse.y;
        } else if (mouse.buttons & Qt.RightButton) {
            if (tasks.showToolTip) {
                toolTip.hide();
            }

            tasks.itemContextMenu(model.Id);
        }
    }

    onReleased: {
        pressX = -1;
        pressY = -1;
    }

    onPositionChanged: {
        if (pressX != -1 && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
            tasks.dragSource = task;
            dragHelper.startDrag(model.MimeType, model.MimeData, model.LauncherUrl, model.DecorationRole);
            pressX = -1;
            pressY = -1;

            return;
        }
    }

    onWheelMoved: TaskTools.activateNextPrevTask(task, wheel.delta < 0)

    PlasmaCore.FrameSvgItem {
        id: frame

        anchors.fill: parent

        imagePath: "widgets/tasks"
        prefix: "normal"
    }

    Item {
        id: iconBox

        anchors {
            left: parent.left
            leftMargin: taskFrame.margins.left
            verticalCenter: parent.verticalCenter
        }

        width: inPopup ? theme.smallIconSize : Math.min(height, parent.width - Layout.horizontalMargins())
        height: Math.min(theme.hugeIconSize,
                         parent.height - (parent.height - Layout.verticalMargins() < theme.smallIconSize ?
                                          Math.min(9, Layout.verticalMargins()) : Layout.verticalMargins()))

        PlasmaCore.IconItem {
            id: icon

            anchors.fill: parent

            visible: !model.IsStartup

            active: task.containsMouse
            enabled: !(model.Minimized && !tasks.showOnlyMinimized)

            source: model.DecorationRole

            onVisibleChanged: {
                if (visible && busyIndicator) {
                    busyIndicator.destroy();
                }
            }
        }

        states: [
            // Using a state transition avoids a binding loop between label.visible and
            // the text label margin, which derives from the icon width.
            State {
                name: "standalone"
                when: !label.visible

                AnchorChanges {
                    target: iconBox
                    anchors.left: undefined
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                PropertyChanges {
                    target: iconBox
                    anchors.leftMargin: 0
                }
            }
        ]
    }

    Tasks.TextLabel {
        id: label

        anchors {
            fill: parent
            leftMargin: taskFrame.margins.left + icon.width + 4
            topMargin: taskFrame.margins.top
            rightMargin: taskFrame.margins.right
            bottomMargin: taskFrame.margins.bottom
        }

        visible: !model.IsLauncher && (parent.width - anchors.leftMargin - anchors.rightMargin) >= (theme.defaultFont.mSize.width * 3)

        enabled: !model.Minimized

        text: model.DisplayRole
        elide: !inPopup
    }

    states: [
        State {
            name: "launcher"
            when: model.IsLauncher

            PropertyChanges {
                target: frame
                prefix: ""
            }
        },
        State {
            name: "hovered"
            when: containsMouse

            PropertyChanges {
                target: frame
                prefix: "hover"
            }
        },
        State {
            name: "attention"
            when: model.DemandsAttention

            PropertyChanges {
                target: frame
                prefix: "attention"
            }
        },
        State {
            name: "minimized"
            when: model.Minimized && !(groupDialog.visible && groupDialog.target == task)

            PropertyChanges {
                target: frame
                prefix: "minimized"
            }
        },
        State {
            name: "active"
            when: model.Active || groupDialog.visible && groupDialog.target == task

            PropertyChanges {
                target: frame
                prefix: "focus"
            }
        }
    ]

    Component.onCompleted: {
        if (model.IsStartup) {
            busyIndicator = Qt.createQmlObject('import org.kde.plasma.components 0.1 as PlasmaComponents; ' +
                'PlasmaComponents.BusyIndicator { anchors.fill: parent; running: true }', iconBox);
        }

        if (model.hasModelChildren) {
            var component = Qt.createComponent("GroupExpanderOverlay.qml");
            component.createObject(task);
        }
    }

    Component.onDestruction: {
        if (groupDialog.visible && groupDialog.groupItemId == model.Id) {
            groupDialog.visible = false;
        }
    }
}
