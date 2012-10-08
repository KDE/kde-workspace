/***********************************************************************************************************************
 * KDE System Tray (Plasmoid)
 * Copyright (C) 2012 ROSA  <support@rosalab.ru>
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
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import Private 0.1

Item {
    id: root_item

    property int icons_size:     24  ///< Size of icons, icons are square i.e. width == height
    property int icons_margins:  4  ///< Margins for icons
    property alias icons_number: list.count  ///< [readonly] Number of icons
    property alias model:    list.model; ///< Model for grid
    property int cell_size: icons_size + 2*icons_margins ///< [readonly] size of grid cell
    property int min_width:   cell_size*11  ///< [readonly] minimum width of component required to show whole grid
    property int min_height:  list.count*cell_size + 2*icons_margins ///< [readonly] minimum height of compontn required to show whole grid

    Component {
        id: delegate_task
        Item {
            id: delegate_root_item
            width: min_width
            height: cell_size

            TrayIcon {
                id: tray_icon
                width: icons_size
                height: icons_size
                anchors { left: parent.left; top: parent.top; margins: icons_margins }
                z: 10 // We place icon over mouse area because icon should receive mouse events
            }

            PlasmaWidgets.Label {
                id: name_item
                anchors { left: tray_icon.right; top: parent.top; bottom: parent.bottom; right: parent.right; leftMargin: icons_margins }
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
                    var pos = mapToItem(list, mouseX, mouseY)
                    // we don't use list.indexAt(pos.x, pos.y) because it has strange behaviour. If user clicks on
                    // embeded applet it will remove from popup and then it will return to popup. Such actions cause
                    // ListView.indexAt return wrong index, at the same time pos.x and pos.y are correct.
                    var index = Math.round(pos.y / cell_size - 0.5)
                    if (index < 0 || index >= list.count)
                        index = -1
                    list.currentIndex = index //list.indexAt(pos.x, pos.y)
                }
            }

        }
    }



    Component {
        id: delegate_highlight
        Item {
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