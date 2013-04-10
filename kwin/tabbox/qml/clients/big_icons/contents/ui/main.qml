/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

Item {
    id: bigIconsTabBox
    property int screenWidth : 0
    property int screenHeight : 0
    property int imagePathPrefix: (new Date()).getTime()
    property int optimalWidth: (icons.iconSize + icons.margins.left + icons.margins.right) * icons.count + background.leftMargin + background.bottomMargin
    property int optimalHeight: icons.iconSize + icons.margins.top + icons.margins.bottom + background.topMargin + background.bottomMargin + 40
    property bool canStretchX: false
    property bool canStretchY: false
    property string maskImagePath: background.maskImagePath
    property double maskWidth: background.centerWidth
    property double maskHeight: background.centerHeight
    property int maskTopMargin: background.centerTopMargin
    property int maskLeftMargin: background.centerLeftMargin
    width: Math.min(Math.max(screenWidth * 0.3, optimalWidth), screenWidth * 0.9)
    height: Math.min(Math.max(screenHeight * 0.05, optimalHeight), screenHeight * 0.5)


    function setModel(model) {
        icons.setModel(model);
    }

    function modelChanged() {
        icons.modelChanged();
    }

    ShadowedSvgItem {
        id: background
        anchors.fill: parent
    }

    IconTabBox {
        id: icons
        iconSize: 128
        height: iconSize + background.topMargin + icons.margins.top + icons.margins.bottom
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            topMargin: background.topMargin
            rightMargin: background.rightMargin
            leftMargin: background.leftMargin
        }
    }
    Item {
        id: captionFrame
        anchors {
            top: icons.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: background.leftMargin
            rightMargin: background.rightMargin
            bottomMargin: background.bottomMargin
        }
        Text {
            function constrainWidth() {
                if (textItem.width > textItem.maxWidth && textItem.width > 0 && textItem.maxWidth > 0) {
                    textItem.width = textItem.maxWidth;
                } else {
                    textItem.width = undefined;
                }
            }
            function calculateMaxWidth() {
                textItem.maxWidth = bigIconsTabBox.width - captionFrame.anchors.leftMargin - captionFrame.anchors.rightMargin - captionFrame.anchors.rightMargin;
            }
            id: textItem
            property int maxWidth: 0
            text: icons.currentItem ? icons.currentItem.data.caption : ""
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: theme.textColor
            elide: Text.ElideMiddle
            font {
                bold: true
            }
            anchors {
                verticalCenter: parent.verticalCenter
                horizontalCenter: parent.horizontalCenter
            }
            onTextChanged: textItem.constrainWidth()
            Component.onCompleted: textItem.calculateMaxWidth()
            Connections {
                target: bigIconsTabBox
                onWidthChanged: {
                    textItem.calculateMaxWidth();
                    textItem.constrainWidth();
                }
            }
        }
    }
}
