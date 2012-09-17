/***********************************************************************************************************************
 * ROSA System Tray (KDE Plasmoid)
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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets


Item {
    id: root_item

    property int icons_size: 24 // size of icons
    property int blink_interval: 1000 // interval of blinking (if status of task is NeedsAttention)
    property variant task: null // task that provides information for item

    property bool     __has_task: task ? true : false
    property string   __icon_name:         __has_task ? task.iconName : ""
    property string   __att_icon_name:     __has_task ? task.attIconName : ""
    property variant  __icon:              __has_task ? task.icon : QIcon("default")
    property variant  __att_icon:          __has_task ? task.attIcon : __getDefaultIcon()
    property string   __overlay_icon_name: __has_task ? task.overlayIconName : ""
    property string   __movie_path:        __has_task ? task.moviePath : ""
    property int      __status:            __has_task ? task.status : TaskStatusUnknown

    // main svg icon redefined by desktop-theme ========================================================================
    property variant svg: PlasmaCore.Svg {
        property variant icon: QIcon(pixmap(__icon_name))
        imagePath: "icons/" + __icon_name.split("-", 1)[0]
    }

    // attention svg icon redefined by desktop-theme ===================================================================
    property variant svg_att: PlasmaCore.Svg {
        property variant icon: QIcon(pixmap(__att_icon_name))
        imagePath: "icons/" + __att_icon_name.split("-", 1)[0]
    }


    Connections {
        target: task

        onChangedShortcut: {
            // update shortcut for icon widget
            if (!icon_widget.action)
                return
            plasmoid.updateShortcutAction(icon_widget.action, task.shortcut)
            icon_widget.action.triggered.disconnect(__onActivatedShortcut) // disconnect old signals
            icon_widget.action.triggered.connect(__onActivatedShortcut)
        }

        onShowContextMenu: plasmoid.showMenu(menu, x, y, root_item)
    }

    function __onActivatedShortcut() {
        __processClick(Qt.LeftButton, icon_widget)
    }


    // Widget for icon =================================================================================================
    PlasmaWidgets.IconWidget {
        id: icon_widget
        action: __has_task ? plasmoid.createShortcutAction(task.objectName + "-" + plasmoid.id) : null
        anchors.fill: parent
        maximumIconSize: Qt.size(parent.width, parent.height)
        visible: false

        onClicked: {
            // we don't process mouse click if action has non-empty shortcut, because icon_widget will trigger action
            if ( !(icon_widget.action && icon_widget.action.globalShortcutEnabled) ) {
                __processClick(Qt.LeftButton, icon_widget)
            }
        }

        Component.onDestruction: {
            var act = icon_widget.action
            icon_widget.action = null
            plasmoid.destroyShortcutAction(act)
        }
    }

    // Overlay icon ====================================================================================================
    Image {
        width: 10  // we fix size of an overlay icon
        height: width
        fillMode: Image.PreserveAspectFit
        anchors { right: parent.right; bottom: parent.bottom }
        smooth: false
        source: "image://icon/" + __overlay_icon_name
        visible: __overlay_icon_name
        z: 2
    }


    // Animation (Movie icon) ==========================================================================================
    AnimatedImage {
        id: animation
        anchors.fill: parent
        playing: false
        visible: false
        smooth: true
        source: __movie_path
        z: 1
    }

    // Timer for blink effect ==========================================================================================
    Timer {
        id: timer_blink
        running: false
        repeat: true
        interval: blink_interval

        property bool is_att_icon: false

        onTriggered: {
            icon_widget.icon = is_att_icon ? __getAttentionIcon() : __getDefaultIcon()
            is_att_icon = !is_att_icon
        }
    }

    // Tooltip =========================================================================================================
    PlasmaCore.ToolTip {
        id: tooltip
        target: icon_widget
        mainText: __has_task ? task.tooltipTitle : ""
        subText: __has_task ? task.tooltipText : ""
        image:   __has_task ? task.tooltipIconName : ""
    }

    // Mouse events handlers ===========================================================================================
    MouseArea {
        id: mouse_area
        anchors.fill: parent
        // if no task passed we don't accept any buttons, if icon_widget is visible we pass left button to it
        acceptedButtons: __has_task ? (( icon_widget.visible ? 0 : Qt.LeftButton) | Qt.RightButton | Qt.MiddleButton) : 0
        enabled: __has_task
        visible: __has_task
        z: 100

        onClicked: 	__processClick(mouse.button, mouse_area)
    }

    WheelArea {
        id: wheel_area
        anchors.fill: parent
        enabled: __has_task
        visible: __has_task
        z: 100

        onScrollVert: task.activateVertScroll(delta)
        onScrollHorz: task.activateHorzScroll(delta)
    }

    // Functions =======================================================================================================
    function __getDefaultIcon() {
        return svg.isValid() ? svg.icon : ( __icon_name ? QIcon(__icon_name) : __icon)
    }

    function __getAttentionIcon() {
        return svg_att.isValid() ? svg_att.icon : (__att_icon_name ? QIcon(__att_icon_name) : __att_icon)
    }

    function __processClick(buttons, item) {
        var pos = plasmoid.popupPosition(item)
        switch (buttons) {
        case Qt.LeftButton:    task.activate1(pos.x, pos.y); break
        case Qt.RightButton:   task.activateContextMenu(pos.x, pos.y); break
        case Qt.MiddleButton:  task.activate2(pos.x, pos.y); break
        }
    }

    // States ==========================================================================================================
    states: [
        // Static icon
        State {
            name: "__STATIC"
            when: __status !== TaskStatusAttention
            PropertyChanges {
                target: timer_blink
                running: false
            }
            PropertyChanges {
                target: icon_widget
                icon: __getDefaultIcon()
                visible: true
            }
            PropertyChanges {
                target: animation
                visible: false
                playing: false
            }
            StateChangeScript {
                script: tooltip.target = icon_widget // binding to property doesn't work
            }
        },
        // Attention icon
        State {
            name: "__BLINK"
            when: __status === TaskStatusAttention && !__movie_path
            PropertyChanges {
                target: icon_widget
                icon: __getAttentionIcon()
                visible: true
            }
            PropertyChanges {
                target: timer_blink
                running: true
                is_att_icon: false
            }
            PropertyChanges {
                target: animation
                visible: false
                playing: false
            }
            StateChangeScript {
                script: tooltip.target = icon_widget
            }
        },
        // Animation icon
        State {
            name: "__ANIM"
            when: __status === TaskStatusAttention && __movie_path
            PropertyChanges {
                target: timer_blink
                running: false
            }
            PropertyChanges {
                target: icon_widget
                icon: __getDefaultIcon()
                visible: false
            }
            PropertyChanges {
                target: animation
                visible: true
                playing: true
            }
            StateChangeScript {
                script: tooltip.target = animation
            }
        }
    ]

}