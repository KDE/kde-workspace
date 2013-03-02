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
    id: textTabBox
    property int screenWidth : 0
    property int screenHeight : 0
    property string longestCaption: ""
    property int optimalWidth: textListView.maxRowWidth
    property int optimalHeight: textListView.rowHeight * textListView.count + background.topMargin + background.bottomMargin
    property bool canStretchX: true
    property bool canStretchY: false
    property string maskImagePath: background.maskImagePath
    property double maskWidth: background.centerWidth
    property double maskHeight: background.centerHeight
    property int maskTopMargin: background.centerTopMargin
    property int maskLeftMargin: background.centerLeftMargin
    width: Math.min(Math.max(screenWidth * 0.2, optimalWidth), screenWidth * 0.8)
    height: Math.min(Math.max(screenHeight * 0.2, optimalHeight), screenHeight * 0.8)
    focus: true

    property int textMargin: 2

    onLongestCaptionChanged: {
        textListView.maxRowWidth = textListView.calculateMaxRowWidth();
    }

    function setModel(model) {
        textListView.model = model;
        textListView.maxRowWidth = textListView.calculateMaxRowWidth();
        textListView.imageId++;
    }

    function modelChanged() {
        textListView.imageId++;
    }

    PlasmaCore.Theme {
        id: theme
    }

    // just to get the margin sizes
    PlasmaCore.FrameSvgItem {
        id: hoverItem
        imagePath: "widgets/viewitem"
        prefix: "hover"
        visible: false
    }

    ShadowedSvgItem {
        id: background
        anchors.fill: parent
    }

    // delegate
    Component {
        id: listDelegate
        Item {
            id: delegateItem
            width: textListView.width
            height: textListView.rowHeight
            Text {
                id: captionItem
                horizontalAlignment: Text.AlignHCenter
                text: caption
                color: theme.textColor
                elide: Text.ElideMiddle
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                    topMargin: hoverItem.margins.top
                    rightMargin: hoverItem.margins.right
                    bottomMargin: hoverItem.margins.bottom
                    leftMargin: hoverItem.margins.left
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textListView.currentIndex = index;
                    textListView.currentIndexChanged(textListView.currentIndex);
                }
            }
        }
    }
    ListView {
        function calculateMaxRowWidth() {
            var width = 0;
            var textElement = Qt.createQmlObject(
                'import QtQuick 1.0;'
                + 'Text {\n'
                + '     text: "' + textTabBox.longestCaption + '"\n'
                + '     visible: false\n'
                + '}',
                textListView, "calculateMaxRowWidth");
            width = Math.max(textElement.width, width);
            textElement.destroy();
            return width + hoverItem.margins.right + hoverItem.margins.left + background.leftMargin + background.rightMargin;
        }
        /**
        * Calculates the height of one row based on the text height and icon size.
        * @return Row height
        **/
        function calcRowHeight() {
            var textElement = Qt.createQmlObject(
                'import QtQuick 1.0;'
                + 'Text {\n'
                + '     text: "Some Text"\n'
                + '     visible: false\n'
                + '}',
                textListView, "calcRowHeight");
            var height = textElement.height;
            textElement.destroy();
            // icon size or two text elements and margins and hoverItem margins
            return height + hoverItem.margins.top + hoverItem.margins.bottom;
        }
        signal currentIndexChanged(int index)
        id: textListView
        objectName: "listView"
        // the maximum text width + icon item width (32 + 4 margin) + margins for hover item + margins for background
        property int maxRowWidth: calculateMaxRowWidth()
        property int rowHeight: calcRowHeight()
        // used for image provider URL to trick Qt into reloading icons when the model changes
        property int imageId: 0
        anchors {
            fill: parent
            topMargin: background.topMargin
            leftMargin: background.leftMargin
            rightMargin: background.rightMargin
            bottomMargin: background.bottomMargin
        }
        clip: true
        delegate: listDelegate
        highlight: PlasmaCore.FrameSvgItem {
            id: highlightItem
            imagePath: "widgets/viewitem"
            prefix: "hover"
            width: textListView.width
        }
        highlightMoveDuration: 250
        boundsBehavior: Flickable.StopAtBounds
    }
    /*
     * Key navigation on outer item for two reasons:
     * @li we have to emit the change signal
     * @li on multiple invocation it does not work on the list view. Focus seems to be lost.
     **/
    Keys.onPressed: {
        if (event.key == Qt.Key_Up) {
            textListView.decrementCurrentIndex();
            textListView.currentIndexChanged(textListView.currentIndex);
        } else if (event.key == Qt.Key_Down) {
            textListView.incrementCurrentIndex();
            textListView.currentIndexChanged(textListView.currentIndex);
        }
    }
}
