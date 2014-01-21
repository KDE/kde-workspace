/***********************************************************************************************************************
 * KDE System Tray (Plasmoid)
 *
 * Copyright (C) 2012 ROSA  <support@rosalab.ru>
 * License: LGPLv2+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **********************************************************************************************************************/

import QtQuick 1.1
import Private 0.1

import "../code/TasksSet.js" as TasksSet
import "../code/main.js" as JS  // stuff & JS additions

Item {
    id: root_item // represents available space on screen

    // This 2 properties must be defined because we use states to set their values
    property int minimumWidth:  JS.MINIMUM_SIZE
    property int minimumHeight: JS.MINIMUM_SIZE

    property int iconSize: Math.min(root_item.width, Math.min(root_item.height, theme.defaultFont.mSize.height < 20 ? 24 : theme.largeIconSize)) //Math.min(root_item.width, Math.min(root_item.height, JS.ICONS_SIZE))

    // Data Models
    property list<ListModel> models: [
        ListModel {id: model_tray},
        ListModel {id: model_popup},
        ListModel {id: model_notifications}
    ]

    Connections {
        target: plasmoid

        onNewTask: {
            // create declarative item
            var component = (task.type === TypeStatusItem ? component_status_item : component_widget)
            var props = {"task": task}
            var item = component.createObject(null, props)
            if (item) {
                var loc = getLocationForTask(task)
                var task_id = plasmoid.getUniqueId(task)
                JS.allTasks[task_id] = task
                var t = JS.tasks[loc].add(task_id, task.category, item)
                models[loc].insert(t.index, {"task": task, "ui_item": item})
            }
        }

        onDeletedTask: {
            var task_id = plasmoid.getUniqueId(task)
            var loc = JS.findLocation(task_id)
            var t = JS.tasks[loc].remove(task_id)
            models[loc].remove(t.index)
            t.data.destroy() // destroy item / we have to destroy it manually because we don't provide parent at initialization
            delete JS.allTasks[task_id]
        }

        onVisibilityPreferenceChanged: {
            // move all tasks to their new location
            for (var task_id in JS.allTasks) {
                var task = JS.allTasks[task_id]
                moveTaskToLocation(task, getLocationForTask(task))
            }
        }

        onActivate: arrow_area.togglePopup()
    }


    Component.onCompleted: {
        // create sets for tasks
        for (var i = 0; i < JS.LOCATIONS_NUMBER; ++i)
            JS.tasks[JS.LOCATIONS[i]] = new TasksSet.TasksSet(JS.CATEGORIES)
    }

    Item {
        id: content_item // represents rectangle containing all visual elements on panel
        anchors.centerIn: parent

        // Notifications area in panel part of tray
        IconsGrid {
            id: notifications_area
            icons_size:    root_item.iconSize
            model: model_notifications
        }

        // Tray area that is in panel
        IconsGrid {
            id: tray_area
            icons_size: root_item.iconSize
            model: model_tray
        }

        // An area that contains arrow
        ArrowArea {
            id: arrow_area
            visible: model_popup.count > 0

            content: IconsList {
                id: popup_area
                icons_size: root_item.iconSize
                model: model_popup
            }
        }
    }

    // Delegates for views =============================================================================================
    Component {
        id: component_status_item

        StatusNotifierItem {
            id: status_item

            blink_interval: JS.BLINK_INTERVAL
            visible: task !== null
            width: root_item.iconSize
            height: width
            anchors.centerIn: parent

            Connections {
                target: task
                onChangedStatus:    moveTaskToLocation(task, getLocationForTask(task))
                onChangedVisibilityPreference: moveTaskToLocation(task, getLocationForTask(task))
            }
        }
    }

    Component {
        id: component_widget

        WidgetItem {
            id: widget_item

            applet: plasmoid
            visible:  task !== null
            width: root_item.iconSize
            height: width
            anchors.centerIn: parent

            Connections {
                target: task
                onChangedStatus:    moveTaskToLocation(task, getLocationForTask(task))
                onChangedVisibilityPreference: moveTaskToLocation(task, getLocationForTask(task))
            }
        }
    }

    // Funtions ========================================================================================================
    function getLocationForTask(task) {
        var loc = getDefaultLocationForTask(task);
        if (loc === JS.LOCATION_TRAY && task.taskId == JS.TASK_NOTIFICATIONS_TYPEID) {
            // redefine location for notifications applet
            return JS.LOCATION_NOTIFICATION;
        }
        return loc;
    }

    /// Returns location depending on status and hide state of task
    function getDefaultLocationForTask(task) {
        var vis_pref = plasmoid.getVisibilityPreference(task);

        if (task.status === NeedsAttention || vis_pref === AlwaysShown) {
            return JS.LOCATION_TRAY;
        }

        if (vis_pref === AlwaysHidden || (task.status !== Active && task.status !== UnknownStatus)) {
            return JS.LOCATION_POPUP;
        }

        return JS.LOCATION_TRAY;
    }

    /// Moves task to specified location
    function moveTaskToLocation(task, loc) {
        var task_id = plasmoid.getUniqueId(task);
        var old_loc = JS.findLocation(task_id);

        if (old_loc === loc) {
            return
        }

        // remove from old location
        var t = JS.tasks[old_loc].remove(task_id);
        models[old_loc].remove(t.index);

        // add to new model
        t = JS.tasks[loc].add(task_id, t.category, t.data);
        models[loc].insert(t.index, {"task": task, "ui_item": t.data});
    }


    // States ==========================================================================================================
    states: [
        State {
            name: "_HORZ" // it is shared state for HORZ and FLOAT
            AnchorChanges {
                target: arrow_area
                anchors { left: tray_area.right; top: content_item.top; bottom: content_item.bottom }
            }
            PropertyChanges {
                target: arrow_area
                // it's strange but if width of arrow area is set to 0 then this may cause crashing of plasma during resising of panel (somewhere in QtDeclarative)
                width: arrow_area.visible ? arrow_area.arrow_size + 2*JS.ARROW_MARGINS : 1
                state: plasmoid.location === TopEdge ? "TOP_EDGE" : "BOTTOM_EDGE"
            }
            PropertyChanges {
                target: popup_area
                state: JS.USE_GRID_IN_POPUP ? "SQR_H" : ""
            }
        },

        State {
            name: "HORZ"
            extend: "_HORZ"
            when: (plasmoid.formFactor === Horizontal)

            AnchorChanges {
                target: notifications_area
                anchors { top: content_item.top; bottom: content_item.bottom; left: content_item.left }
            }
            PropertyChanges {
                target: notifications_area
                state: "HORZ"
                width: notifications_area.min_width
            }
            AnchorChanges {
                target: tray_area
                anchors { left: notifications_area.right; top: content_item.top; bottom: content_item.bottom }
            }
            PropertyChanges {
                target: tray_area
                state: "HORZ"
                width: tray_area.min_width
            }
            PropertyChanges {
                target: content_item
                width: notifications_area.width + tray_area.width + arrow_area.width
                height: root_item.height
            }
            PropertyChanges {
                target: root_item
                minimumWidth: content_item.width
                minimumHeight: JS.MINIMUM_SIZE
            }
        },

        State {
            name: "VERT"
            when: (plasmoid.formFactor === Vertical)

            AnchorChanges {
                target: notifications_area
                anchors { left: content_item.left; right: content_item.right; top: content_item.top }
            }
            PropertyChanges {
                target: notifications_area
                state: "VERT"
                height: notifications_area.min_height
            }
            AnchorChanges {
                target: tray_area
                anchors { left: content_item.left; right: content_item.right; top: notifications_area.bottom }
            }
            PropertyChanges {
                target: tray_area
                state: "VERT"
                height: tray_area.min_height
            }
            AnchorChanges {
                target: arrow_area
                anchors { left: content_item.left; right: content_item.right; top: tray_area.bottom }
            }
            PropertyChanges {
                target: arrow_area
                height: arrow_area.visible ? arrow_area.arrow_size + 2*JS.ARROW_MARGINS : 1
                state: plasmoid.location === LeftEdge ? "LEFT_EDGE" : "RIGHT_EDGE"
            }
            PropertyChanges {
                target: popup_area
                state: JS.USE_GRID_IN_POPUP ? "SQR_V" : ""
            }
            PropertyChanges {
                target: content_item
                width: root_item.width
                height: notifications_area.height + tray_area.height + arrow_area.height
            }
            PropertyChanges {
                target: root_item
                minimumWidth: JS.MINIMUM_SIZE
                minimumHeight: content_item.height
            }
        },

        State {
            name: "FLOAT"
            extend: "_HORZ"
            when: (plasmoid.formFactor === Floating)

            PropertyChanges {
                target: notifications_area
                state: "SQR_H"
                width: notifications_area.min_width
                height: notifications_area.min_height
            }
            AnchorChanges {
                target: notifications_area
                anchors { left: content_item.left; verticalCenter: content_item.verticalCenter }
            }
            AnchorChanges {
                target: tray_area
                anchors { left: notifications_area.right; verticalCenter: content_item.verticalCenter }
            }
            PropertyChanges {
                target: tray_area
                state: "SQR_V"
                width: tray_area.min_width
                height: tray_area.min_height
            }
            PropertyChanges {
                target: content_item
                width: notifications_area.width + tray_area.width + arrow_area.width
                height: Math.max(notifications_area.min_height, tray_area.min_height, arrow_area.arrow_size)
            }
            PropertyChanges {
                target: root_item
                minimumWidth: content_item.width
                minimumHeight: content_item.height
            }
        }
    ]


}
