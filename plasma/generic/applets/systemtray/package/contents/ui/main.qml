/***********************************************************************************************************************
 * KDE System Tray (KDE Plasmoid)
 * Copyright (C) 2011-2012 ROSA  <support@rosalab.ru>
 * License: GPLv3+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **********************************************************************************************************************/

import QtQuick 1.1
import Private 0.1

import "tasks.js" as Tasks

Item {
    id: root_item

    // Constants
    property int __icons_size: 24 // Size of icons, icons are square i.e. width == height
    property int __icons_margins: 2 // Margins for icons
    property int __minimum_size: 30 // minimum size of widget
    property int __arrow_size: 12 // size of an arrow for popup
    property int __arrow_margins: 5 // margins for an arrow
    property int __blink_interval: 750 // time interval of blinking
    property string __notification_task_name: "notifications"

    // This 2 properties must be defined because we use states to set their values
    property int minimumWidth:  __minimum_size
    property int minimumHeight: __minimum_size

    // Data Models
    property list<ListModel> __models: [
        ListModel {id: model_tray},
        ListModel {id: model_popup},
        ListModel {id: model_notifications}
    ]

    Connections {
        target: tasks_pool

        onNewTask: {
            // create declarative item
            var component = (task_type === TaskTypeStatusItem ? component_status_item : component_widget)
            var props = (task_type === TaskTypeStatusItem ? {"task": task.task} : {}) // some properties we set later
            var item = component.createObject(null, props)
            if (item) {
                var loc = __getLocationForTask(task)
                var model = __models[loc]
                Tasks.addTask(task_id, model.count, loc, item)
                model.append({"ui_task": task, "ui_item": item})
            }
        }

        onDeletedTask: {
            var task = Tasks.tasks[task_id]
            var model = __models[task.location]
            model.remove(task.model_index)
            task.item.destroy() // destroy item / we have to destroy it manually because we don't provide parent at initialization
            Tasks.removeTask(task_id)
        }
    }

    Connections {
        target: plasmoid
        onActivated: arrow_area.togglePopup()
    }

    // Notifications area in panel part of tray ========================================================================
    IconsGrid {
        id: notifications_area
        icons_size:    __icons_size
        icons_margins: __icons_margins
        model: model_notifications
        delegate: delegate_task
    }

    // Tray area that is in panel ======================================================================================
    IconsGrid {
        id: tray_area
        icons_size: __icons_size
        icons_margins: __icons_margins
        model: model_tray
        delegate: delegate_task
    }

    // An area that contains arrow =====================================================================================
    ArrowArea {
        id: arrow_area
        content: IconsGrid {
            id: popup_area
            icons_size:    __icons_size
            icons_margins: __icons_margins
            width: popup_area.min_width
            height: popup_area.min_height
            anchors.centerIn: parent
            model: model_popup
            delegate: delegate_task
        }
    }

    // Delegates for views =============================================================================================
    Component {
        id: component_status_item

        StatusNotifierItem {
            id: status_item
            icons_size: __icons_size
            blink_interval: __blink_interval
            task: null
            visible: task !== null
            width: __icons_size
            height: width
            anchors.centerIn: parent
        }
    }

    Component {
        id: component_widget

        WidgetItem {
            id: widget_item
            widget: null
            visible:  widget !== null
            width: __icons_size
            height: width
            anchors.centerIn: parent
        }
    }

    // Delegate for views ==============================================================================================
    Component {
        id: delegate_task

        Item {
            id: delegate_task_item
            width:  GridView.view.cellWidth
            height: GridView.view.cellHeight

            Connections {
                target: ui_task
                onChangedStatus:    __moveTaskToLocation(ui_task.taskId, __getLocationForTask(ui_task))
                onChangedHideState: __moveTaskToLocation(ui_task.taskId, __getLocationForTask(ui_task))
            }

            Component.onCompleted: {
                ui_item.parent = delegate_task_item
                // reset properties
                if (ui_task.type !== TaskTypeStatusItem && !ui_item.widget) {
                    ui_item.widget = ui_task.widget
                }
            }
        }
    }

    // Funtions ========================================================================================================
    function __getLocationForTask(task) {
        var loc = __getDefaultLocationForTask(task)
        if (loc === Tasks.LOCATION.TRAY && task.name() == __notification_task_name)
            return Tasks.LOCATION.NOTIFICATIONS // redefine location for notifications applet
        return loc
    }

    /// Returns location depending on status and hide state of task
    function __getDefaultLocationForTask(task) {
        if (task.status === TaskStatusAttention || task.hideState === TaskHideStateShown) return Tasks.LOCATION.TRAY
        if (task.hideState === TaskHideStateHidden || (task.status !== TaskStatusActive && task.status !== TaskStatusUnknown)) {
            return Tasks.LOCATION.POPUP
        }
        return Tasks.LOCATION.TRAY
    }

    /// Moves task to specified location
    function __moveTaskToLocation(task_id, loc) {
        var task = Tasks.tasks[task_id]
        if (!task || task.location === loc)
            return
        // remove from old location
        var model = __models[task.location]
        model.remove(task.model_index)
        Tasks.unbind(task)
        // add to new model
        model = __models[loc]
        task.model_index = model.count
        task.location = loc
        model.append({"ui_task": tasks_pool.tasks[task_id], "ui_item": task.item})
    }


    // States ==========================================================================================================
    states: [
        State {
            name: "HORZ"
            when: (plasmoid.formFactor === Horizontal)

            AnchorChanges {
                target: notifications_area
                anchors { top: root_item.top; bottom: root_item.bottom; left: root_item.left; right: undefined }
            }
            PropertyChanges {
                target: notifications_area
                state: "HORZ"
                width: notifications_area.min_width
            }
            AnchorChanges {
                target: tray_area
                anchors { top: root_item.top; bottom: root_item.bottom; left: notifications_area.right; right: undefined }
            }
            PropertyChanges {
                target: tray_area
                state: "HORZ"
                width: tray_area.min_width
            }
            AnchorChanges {
                target: arrow_area
                anchors { left: tray_area.right; top: root_item.top; bottom: root_item.bottom; right: undefined }
            }
            PropertyChanges {
                target: arrow_area
                width: root_item.__arrow_size
                anchors { leftMargin: __arrow_margins; rightMargin: __arrow_margins; topMargin: 0; bottomMargin: 0 }
                state: plasmoid.location === TopEdge ? "TOP_EDGE" : "BOTTOM_EDGE"
            }
            PropertyChanges {
                target: popup_area
                state: "SQR_H"
            }
            PropertyChanges {
                target: root_item
                minimumWidth: notifications_area.width + tray_area.width + arrow_area.width + 2*__arrow_margins
                minimumHeight: __minimum_size
            }
        },

        State {
            name: "VERT"
            when: (plasmoid.formFactor === Vertical)

            AnchorChanges {
                target: notifications_area
                anchors { left: root_item.left; right: root_item.right; top: root_item.top; bottom: undefined}
            }
            PropertyChanges {
                target: notifications_area
                state: "VERT"
                height: notifications_area.min_height
            }
            AnchorChanges {
                target: tray_area
                anchors { left: root_item.left; right: root_item.right; top: notifications_area.bottom; bottom: undefined }
            }
            PropertyChanges {
                target: tray_area
                state: "VERT"
                height: tray_area.min_height
            }
            AnchorChanges {
                target: arrow_area
                anchors { top: tray_area.bottom; left: root_item.left; right: root_item.right; bottom: undefined }
            }
            PropertyChanges {
                target: arrow_area
                height: root_item.__arrow_size
                anchors { leftMargin: 0; rightMargin: 0; topMargin: __arrow_margins; bottomMargin: __arrow_margins }
                state: plasmoid.location === LeftEdge ? "LEFT_EDGE" : "RIGHT_EDGE"
            }
            PropertyChanges {
                target: popup_area
                state: "SQR_V"
            }
            PropertyChanges {
                target: root_item
                minimumWidth: __minimum_size
                minimumHeight: notifications_area.height + tray_area.height + arrow_area.height + 2*__arrow_margins
            }
        }
    ]


}
