/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012 Marco Martin <mart@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents


Item {
    id: sectionDelegate

    width: parent.width
    height: childrenRect.height

    PlasmaCore.SvgItem {
        anchors {
            left: parent.left
            right: parent.right
        }
        height: lineSvg.elementSize("horizontal-line").height

        visible: sectionDelegate.y > 0
        svg: lineSvg
        elementId: "horizontal-line"
    }
    PlasmaComponents.Label {
        anchors {
            left: parent.left
            right: parent.right
        }

        y: 2
        opacity: 0.6
        text: section
        horizontalAlignment: Text.AlignHCenter
    }
} // sectionDelegate
