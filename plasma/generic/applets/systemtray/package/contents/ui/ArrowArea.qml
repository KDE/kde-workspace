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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.extras 0.1 as PlasmaExtras

Item {
    id: arrow_area

    property variant content; ///< content of popup dialog
    property int arrow_size: 12 // size of an icon

    MouseArea {
        anchors.fill: parent
        onClicked: togglePopup()
        onPressed: PlasmaExtras.PressedAnimation { targetItem: arrow_widget }
        onReleased: PlasmaExtras.ReleasedAnimation { targetItem: arrow_widget }
    }

    PlasmaCore.SvgItem {

        id: arrow_widget

        anchors.centerIn: parent
        width: arrow_size
        height: width

        svg: PlasmaCore.Svg { imagePath: "widgets/arrows" }
        elementId: "left-arrow"
    }

    // Tooltip for arrow -----------------------------------------------------------------------------------------------
    PlasmaCore.ToolTip {
        id: arrow_tooltip
        target: arrow_widget
        subText: dialog.visible ? i18n("Hide icons") : i18n("Show hidden icons")
    }

    // Popup dialog (window) -------------------------------------------------------------------------------------------
    PlasmaCore.Dialog {
        id: dialog
        visible: false
        mainItem: content
        location: plasmoid.location
        windowFlags: Qt.WindowStaysOnTopHint

        onActiveWindowChanged: dialog.visible = activeWindow  // hide window if it deactivates


        // We have to move dialog if its size is changed
        onHeightChanged:  updatePosition()
        onWidthChanged:   updatePosition()

        onVisibleChanged: {
            if (visible) {
                if (dialog.windowId)
                    plasmoid.hideFromTaskbar(dialog.windowId)
                updatePosition()
            }
        }

        function updatePosition() {
            var pos = dialog.popupPosition(arrow_area, Qt.AlignCenter)
            x = pos.x
            y = pos.y
        }
    }


    function togglePopup() {
        if (dialog.visible) {
            dialog.visible = false
            return
        }
        dialog.visible = true
        dialog.activateWindow()
    }


    states: [
        State {
            name: "LEFT_EDGE"
            PropertyChanges {
                target: arrow_widget
                elementId: dialog.visible ? "left-arrow" : "right-arrow"
            }
        },

        State {
            name: "RIGHT_EDGE"
            PropertyChanges {
                target: arrow_widget
                elementId: dialog.visible ? "right-arrow" : "left-arrow"
            }
        },

        State {
            name: "TOP_EDGE"
            PropertyChanges {
                target: arrow_widget
                elementId: dialog.visible ? "up-arrow" : "down-arrow"
            }
        },

        State {
            name: "BOTTOM_EDGE"
            PropertyChanges {
                target: arrow_widget
                elementId: dialog.visible ? "down-arrow" : "up-arrow"
            }
        }
    ]

}
