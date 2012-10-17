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
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import Private 0.1

import "IconsList.js" as IconsListJS

Item {
    id: root_item

    property int icons_size:     24  ///< Size of icons, icons are square i.e. width == height
    property int icons_margins:  4  ///< Margins for icons
    property alias icons_number: list.count  ///< [readonly] Number of icons
    property alias model:    list.model; ///< Model for grid
    property int cell_size: icons_size + 2*icons_margins ///< [readonly] size of grid cell
    property int min_width:   cell_size + icons_margins + __max_name_width   ///< [readonly] minimum width of component required to show whole grid
    property int min_height:  list.count*cell_size ///< [readonly] minimum height of compontn required to show whole grid

    property int __max_name_width: 0

    Component {
        id: delegate_task
        Item {
            id: delegate_root_item
            width: min_width
            height: cell_size

            Item {
                id: tray_icon
                anchors { left: parent.left; top: parent.top; }
                width: cell_size
                height: width
                z: 10 // We place icon over mouse area because icon should receive mouse events

                TrayIcon {
                    anchors.centerIn: parent
                    width: icons_size
                    height: width
                }
            }

            PlasmaWidgets.Label {
                id: name_item
                anchors { left: tray_icon.right; top: parent.top; bottom: parent.bottom }
                alignment: Qt.AlignLeft | Qt.AlignVCenter
                wordWrap: false
                textSelectable: false
                text: ui_task.name
                z: -10 // We place label under mouse area to be able to handle mouse events
            }

            MouseRedirectArea {
                id: redirect_area
                anchors.fill: parent
                z: 0

                // we redirect some events to IconWidget or applet
                target: ui_task.widget ? ui_task.widget : ui_item.getIconWidget()
                applet: plasmoid.applet
                isWidget: ui_task.widget != null

                // Next events we process manually
                onClickMiddle: ui_item.click(Qt.MiddleButton)
                onClickRight: ui_item.click(Qt.RightButton)
                onScrollVert: ui_item.scrollVert(delta)
                onScrollHorz: ui_item.scrollHorz(delta)
                onChangedMousePos: {
                    var pos = mapToItem(list.contentItem, mouseX, mouseY)
                    list.currentIndex = list.indexAt(pos.x, pos.y)
                }
            }

            ListView.onAdd: {
                var text_width = name_item.width
                IconsListJS.tasks[delegate_root_item] = text_width
                if (text_width > __max_name_width) {
                    __max_name_width = text_width
                }
            }

            ListView.onRemove: {
                delete IconsListJS.tasks[delegate_root_item]
                __max_name_width = IconsListJS.findMax()  // recalculate width of maximum name
            }

            Connections {
                target: ui_task
                onChangedName: {
                    // if name is changed => we should recalculate width of popup
                    IconsListJS.tasks[delegate_root_item] = name_item.width
                    __max_name_width = IconsListJS.findMax()
                }
            }

        }
    }



    Component {
        id: delegate_highlight
        Item {
            height: cell_size
            width: min_width

            PlasmaWidgets.ItemBackground {
                anchors.fill: parent
            }
        }
    }

    ListView {
        id: list
        anchors.centerIn: parent
        width:  min_width
        height: min_height

        interactive: false
        delegate: delegate_task
        highlight: delegate_highlight
        highlightFollowsCurrentItem: true
        highlightMoveSpeed: -1
        highlightMoveDuration: 250
        spacing: 0
        snapMode: ListView.SnapToItem
    }
}