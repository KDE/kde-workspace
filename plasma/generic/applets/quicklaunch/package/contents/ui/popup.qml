/***************************************************************************
 *   Copyright (C) 2012 by Ingomar Wesp <ingomar@wesp.name>                *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************/
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1 as QtExtraComponents

import org.kde.plasma.quicklaunch 1.0 as Quicklaunch

Item {
    id: main
    property alias model: list.model
    property alias count: list.count

    PlasmaCore.Svg {
       id: arrowsSvg
       imagePath: "widgets/arrows"
    }

     PlasmaCore.SvgItem {
        id: popupTrigger

        anchors.right: parent.right

        svg: arrowsSvg
        elementId: {
            switch(plasmoid.location) {
                case TopEdge:
                    return dialog.visible ? "up-arrow" : "down-arrow";
                case LeftEdge:
                    return dialog.visible ? "left-arrow" : "right-arrow";
                case RightEdge:
                    return dialog.visible ? "right-arrow" : "left-arrow";
                default:
                    return dialog.visible ? "down-arrow" : "up-arrow";
            }
        }

        width: 16;
        height: 16;

        MouseArea {
            anchors.fill: parent
            onClicked: {
                var popupPosition = dialog.popupPosition(popupTrigger);
                dialog.x = popupPosition.x;
                dialog.y = popupPosition.y;
                dialog.visible = !dialog.visible;
            }
        }
    }

    PlasmaCore.Dialog {
        id: dialog
        mainItem: list
        visible: main.visible

        windowFlags: Qt.X11BypassWindowManagerHint
    }

    ListView {
        id: list
        width: 160
        height: count * (24) + (count - 1) * spacing
        spacing: 8

        model: Quicklaunch.LauncherListModel {}

        delegate: Row {

            spacing: 4

            QtExtraComponents.QIconItem {
                id: iconItem
                width: 24
                height: 24

                icon: QIcon(iconSource)
            }

            Text {
                anchors.verticalCenter: iconItem.verticalCenter
                text: display
            }
        }
    }
}
