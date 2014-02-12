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

ColumnLayout {
    id: root
    property alias cfg_Color: picker.color
    property string cfg_Image
    property int cfg_FillMode
    property var cfg_SlidePaths: ""

    spacing: units.largeSpacing / 2

    Wallpaper.Image {
        id: imageWallpaper
        onSlidePathsChanged: cfg_SlidePaths = slidePaths
    }

    onCfg_SlidePathsChanged: {
        imageWallpaper.slidePaths = cfg_SlidePaths
    }

    //Rectangle { color: "orange"; x: formAlignment; width: formAlignment; height: 20 }

    Row {
        //x: formAlignment - positionLabel.paintedWidth
        spacing: units.largeSpacing / 2
        QtControls.Label {
            id: positionLabel
            width: formAlignment - units.largeSpacing
            anchors {
                verticalCenter: resizeComboBox.verticalCenter
            }
            text: i18nc("Label for positioning combo", "Positioning:")
            horizontalAlignment: Text.AlignRight
        }
        QtControls.ComboBox {
            id: resizeComboBox
            property int textLength: 24
            width: theme.mSize(theme.defaultFont).width * textLength
            model: [
                        {
                            'label': i18n("Scaled & Cropped"),
                            'fillMode': Image.PreserveAspectCrop
                        },
                        {
                            'label': i18n("Scaled"),
                            'fillMode': Image.Stretch
                        },
                        {
                            'label': i18n("Scaled, Keep Proportions"),
                            'fillMode': Image.PreserveAspectFit
                        },
                        {
                            'label': i18n("Centered"),
                            'fillMode': Image.Pad
                        },
                        {
                            'label': i18n("Tiled"),
                            'fillMode': Image.Tile
                        }
                    ]

            textRole: "label"
            onCurrentIndexChanged: cfg_FillMode = model[currentIndex]["fillMode"]
            Component.onCompleted: setMethod();

            function setMethod() {
                for (var i = 0; i < model.length; i++) {
                    if (model[i]["fillMode"] == wallpaper.configuration.FillMode) {
                        resizeComboBox.currentIndex = i;
                        var tl = model[i]["label"].length;
                        //resizeComboBox.textLength = Math.max(resizeComboBox.textLength, tl+5);
                    }
                }
            }
        }
    }

    ColorPicker {
        id: picker
        visible: ~[2,3].indexOf(resizeComboBox.currentIndex)
    }

    //TODO: this should be shown instead of the main one when in slideshow mode
    /*QtControls.ScrollView {
        anchors {
            left: parent.left
            right: parent.right
        }
        height: units.gridUnit * 10
        ListView {
            id: slidePathsView
            model: imageWallpaper.slidePaths
            delegate: QtControls.Label {
                text: modelData
                width: slidePathsView.width
            }
        }
    }
    QtControls.Button {
        text: i18n("Add Folder")
        onClicked: imageWallpaper.showAddSlidePathsDialog()
    }*/

    QtControls.ScrollView {
        Layout.fillHeight: true;
        anchors {
            left: parent.left
            right: parent.right
        }
        height: units.gridUnit * 30

        frameVisible: false
        highlightOnFocus: true;

        GridView {
            id: wallpapersGrid
            model: imageWallpaper.wallpaperModel
            currentIndex: -1

            highlight: Rectangle {
                radius: 3
                color: syspal.highlight
            }
            delegate: MouseArea {
                id: wallpaperDelegate
                width: wallpapersGrid.cellWidth
                height: wallpapersGrid.cellHeight

                Column {
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }
                    QPixmapItem {
                        anchors.horizontalCenter: parent.horizontalCenter
                        height: wallpaperDelegate.height - wallpaperName.height -20
                        width: height
                        pixmap: model.screenshot
                    }
                    QtControls.Label {
                        id: wallpaperName
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: model.display
                        wrapMode: Text.Wrap
                        horizontalAlignment: Text.AlignHCenter
                        color: (wallpapersGrid.currentIndex == index) ? syspal.highlightedText : syspal.text
                    }
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
            Timer {
                id: makeCurrentTimer
                interval: 100
                repeat: false
                property string pendingIndex
                onTriggered: {
                    wallpapersGrid.currentIndex = pendingIndex
                }
            }
        }
    }
}