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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras
import Private 0.1

import "../code/IconsList.js" as IconsListJS

MouseArea {
    id: root_item

    property int icons_size:     24  ///< Size of icons, icons are square i.e. width == height
    property int icons_margins:  icons_size/6  ///< Margins for icons
    property alias icons_number: repeater.count  ///< [readonly] Number of icons
    property alias model:    repeater.model; ///< Model for grid
    property int cell_size: icons_size + 2*icons_margins ///< [readonly] size of grid cell

    //Those properties are used by PlasmaCore.Dialog for size hints
    property int extraHorizMargins: delegate_highlight.marginHints.right
    property int minimumWidth: extraHorizMargins + layoutColumn.childrenRect.width ///< [readonly] minimum width of component required to show whole grid
    property int minimumHeight:layoutColumn.childrenRect.height ///< [readonly] minimum height of compontn required to show whole grid
    property int maximumWidth: minimumWidth
    property int maximumHeight: minimumHeight

    hoverEnabled: true

    Component {
        id: delegate_task

        MouseRedirectArea {
            id: delegate_root_item
            width: Math.max(minimumWidth - extraHorizMargins, childrenRect.width)
            height: childrenRect.height

            // we redirect some events to IconWidget or applet
            target: task.type == TypeStatusItem ? ui_item.getMouseArea() : task
            applet: plasmoid

            // Next events we process manually
            onClickMiddle: ui_item.click(Qt.MiddleButton)
            onClickRight: ui_item.click(Qt.RightButton)
            onScrollVert: ui_item.scrollVert(delta)
            onScrollHorz: ui_item.scrollHorz(delta)
            onEntered: {
                delegate_highlight.y = delegate_root_item.y
            }

            Row {
                Item {
                    id: tray_icon

                    width: cell_size
                    height: width

                    TrayIcon {
                        anchors.centerIn: parent
                        width: icons_size
                        height: width
                    }
                }

                PlasmaComponents.Label {
                    id: name_item
                    anchors.verticalCenter: tray_icon.verticalCenter
                    wrapMode: Text.NoWrap
                    text: task.name
                }
            }


            Component.onCompleted: {
                var text_width = name_item.width
                IconsListJS.tasks[delegate_root_item] = text_width
            }

            Component.onDestruction: {
                delete IconsListJS.tasks[delegate_root_item]
            }

            Connections {
                target: task
                onChangedName: {
                    // if name is changed => we should recalculate width of popup
                    IconsListJS.tasks[delegate_root_item] = name_item.width
                }
            }

        }

    }

    PlasmaComponents.Highlight {
        id: delegate_highlight
        height: cell_size
        width: minimumWidth
        opacity: root_item.containsMouse

        Behavior on opacity {
            NumberAnimation {
                duration: 150
                easing: Easing.InOutQuad
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: 250
                easing: Easing.InOutQuad
            }
        }
    }


    Column {
        id: layoutColumn
        spacing: 0
        Repeater {
            id: repeater
            delegate: delegate_task
        }

    }

    Component.onCompleted: {
        console.log("========================> " + " " + minimumWidth + " " + layoutColumn.childrenRect.width);
    }
}
