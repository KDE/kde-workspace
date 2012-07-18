/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: root

    property int minimumWidth: 176
    property int minimumHeight: 88

    anchors.fill: parent

    Repeater {
        id: repeater
        model: pager.model

        PlasmaCore.FrameSvgItem {
            id: frameSvg
            x: model.x
            y: model.y
            width: model.width
            height: model.height
            imagePath: "widgets/pager"
            prefix: mouseArea.containsMouse ?
                        "hover" : (index === pager.currentDesktop-1 ? "active" : "normal")

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
            }

            Item {
                id: clipRect
                x: 1
                y: 1
                width: frameSvg.width - 2
                height: frameSvg.height - 2
                clip: true

                Repeater {
                    model: windows

                    Rectangle {
                        x: model.x
                        y: model.y
                        width: model.width
                        height: model.height
                        color: model.active ? "green" : "red"
                        opacity: 0.5
                    }
                }
            }
        }
    }
}
