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
    id: arrow_area

    property variant content; ///< content of popup dialog
    property int __arrow_size: Math.min(width, height) // for internal use only

    MouseArea {
        anchors.fill: parent
        onClicked: togglePopup()
        z: -1 // avoid overlap of arrow_widget
    }

    PlasmaWidgets.IconWidget {
        id: arrow_widget
        width:  __arrow_size
        height: width
        preferredIconSize: Qt.size(__arrow_size, __arrow_size)
        maximumIconSize:   Qt.size(__arrow_size, __arrow_size)
        minimumIconSize:   Qt.size(__arrow_size, __arrow_size)
        anchors.centerIn: parent
        icon: QIcon(svg.pixmap(svg_element_id))

        property string svg_element_id: "left-arrow"  // name of element in svg file

        property variant svg: PlasmaCore.Svg {
            imagePath: "widgets/arrows"
        }

        onClicked: togglePopup()
    }

    // Tooltip for arrow -----------------------------------------------------------------------------------------------
    PlasmaCore.ToolTip {
        id: arrow_tooltip
        target: arrow_widget
        subText: dialog.visible ? i18n("Hide icons") : i18n("Show hidden icons")
    }

    // Popup dialog (window) -------------------------------------------------------------------------------------------
    Dialog {
        id: dialog
        visible: false
        item: content
        location: plasmoid.location

        onChangedActive: {
            dialog.visible = active  // hide window if it deactivates
        }
    }


    function togglePopup() {
        if (dialog.visible) {
            dialog.visible = false
            return
        }
        var pos = plasmoid.popupPosition(arrow_area, Qt.size(dialog.width, dialog.height), Qt.AlignCenter)
        dialog.x = pos.x
        dialog.y = pos.y
        dialog.visible = true
        dialog.activate()
    }


    states: [
        State {
            name: "LEFT_EDGE"
            PropertyChanges {
                target: arrow_widget
                svg_element_id: (dialog.visible ? "left-arrow" : "right-arrow")
            }
        },

        State {
            name: "RIGHT_EDGE"
            PropertyChanges {
                target: arrow_widget
                svg_element_id: dialog.visible ? "right-arrow" : "left-arrow"
            }
        },

        State {
            name: "TOP_EDGE"
            PropertyChanges {
                target: arrow_widget
                svg_element_id: dialog.visible ? "up-arrow" : "down-arrow"
            }
        },

        State {
            name: "BOTTOM_EDGE"
            PropertyChanges {
                target: arrow_widget
                svg_element_id: dialog.visible ? "down-arrow" : "up-arrow"
            }
        }
    ]

}
