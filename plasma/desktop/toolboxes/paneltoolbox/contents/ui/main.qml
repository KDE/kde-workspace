/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: main

    width: toolBoxButton.width
    height: toolBoxButton.height

    anchors {
        left: undefined
        top: undefined
        right: (plasmoid.formFactor == 3) ? undefined : parent.right
        bottom: (plasmoid.formFactor == 3) ? parent.bottom : undefined
        verticalCenter: (plasmoid.formFactor == 3) ? undefined : parent.verticalCenter
        horizontalCenter: (plasmoid.formFactor == 3) ? parent.horizontalCenter : undefined
    }

    PlasmaCore.Svg {
        id: toolBoxSvg
        imagePath: "widgets/toolbox"
    }

    PlasmaCore.SvgItem {
        id: toolBoxButton
        svg: toolBoxSvg
        width: naturalSize.width
        height: naturalSize.height
        elementId: {
            if (plasmoid.formFactor == 3) {
                return "panel-south"
            } else {
                return "panel-east"
            }
        }
        MouseArea {
            id: mouseArea
            anchors {
                fill: parent
                topMargin: (plasmoid.formFactor == 3) ? 5 : 0
                leftMargin: (plasmoid.formFactor == 3) ? 0 : 5
            }
            hoverEnabled: true
            onClicked: {
                plasmoid.action("configure").trigger()
            }
            PlasmaCore.IconItem {
                anchors.fill: parent
                source: "plasma"
                enabled: mouseArea.containsMouse
            }
        }
    }
}
