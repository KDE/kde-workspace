/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
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
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.0
import org.kde.plasma.wallpapers.image 2.0 as Wallpaper
import org.kde.qtextracomponents 2.0

MouseArea {
    id: wallpaperDelegate
    width: wallpapersGrid.cellWidth
    height: wallpapersGrid.cellHeight

    hoverEnabled: true

    Column {
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        QPixmapItem {
            anchors.horizontalCenter: parent.horizontalCenter
            //height: wallpaperDelegate.height - wallpaperName.height -20
            height: wallpaperDelegate.height * 0.8
            width: height
            pixmap: model.screenshot
            scale: wallpaperDelegate.containsMouse ? 1/0.7 : 1.0
            z:  wallpaperDelegate.containsMouse ? 1 : 0
            Rectangle {
                anchors.fill: parent
                border.width: (wallpapersGrid.currentIndex == index) ? units.smallSpacing : 0
                border.color: (wallpapersGrid.currentIndex == index) ? syspal.highlight : "transparent"
                color: "transparent"
            }
            Behavior on scale {
                SequentialAnimation {
                    ScriptAction { script: parent.z = 99 }
                    PropertyAnimation { duration: units.shortDuration } }
            }
        }
//         QtControls.Label {
//             id: wallpaperName
//             anchors {
//                 left: parent.left
//                 right: parent.right
//             }
//             text: model.display
//             wrapMode: Text.Wrap
//             horizontalAlignment: Text.AlignHCenter
//             color: (wallpapersGrid.currentIndex == index) ? syspal.highlightedText : syspal.text
//         }
    }
    onClicked: {
        cfg_Image = model.path
        wallpapersGrid.currentIndex = index
    }
    Component.onCompleted: {
        if (cfg_Image == model.path) {
            makeCurrentTimer.pendingIndex = model.index
            makeCurrentTimer.restart()
        }
    }
}