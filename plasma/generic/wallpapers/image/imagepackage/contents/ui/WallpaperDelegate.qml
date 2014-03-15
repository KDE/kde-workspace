/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import org.kde.qtextracomponents 2.0

MouseArea {
    id: wallpaperDelegate

    width: wallpapersGrid.cellWidth
    height: wallpapersGrid.cellHeight

    property bool selected: (wallpapersGrid.currentIndex == index)

    onSelectedChanged: cfg_Image = model.path

    Column {
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        QPixmapItem {
            anchors.horizontalCenter: parent.horizontalCenter
            height: wallpaperDelegate.height + 1
            width: wallpaperDelegate.width + 1
            smooth: true
            pixmap: model.screenshot
            fillMode: QPixmapItem.PreserveAspectCrop

            Rectangle {
                opacity: selected ? 1.0 : 0
                anchors.fill: parent
                border.width: units.smallSpacing * 2
                border.color: syspal.highlight
                color: "transparent"
                Behavior on opacity {
                    PropertyAnimation {
                        duration: units.longDuration
                        easing.type: Easing.OutQuad
                    }
                }
            }
        }
    }

    onClicked: {
        wallpapersGrid.currentIndex = index
    }

    Component.onCompleted: {
        if (cfg_Image == model.path) {
            makeCurrentTimer.pendingIndex = model.index
            makeCurrentTimer.restart()
        }
    }
}