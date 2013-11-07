/***************************************************************************
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.qtextracomponents 2.0 as QtExtraComponents

import org.kde.private.systemtray 2.0 as SystemTray


QtExtraComponents.MouseEventListener {
    id: compactRepresenation

    // TODO: vertical formfactor
    //property int minimumWidth: (1.5 + systrayhost.shownTasks.length) * (root.itemSize + itemSpacing) + (2 * itemSpacing)
    property int minimumWidth: !vertical ? computeDimension() : undefined
    property int minimumHeight: vertical ? computeDimension() : undefined
    property int maximumWidth: !vertical ? minimumWidth : computeDimension()
    property int maximumHeight: vertical ? minimumWidth : computeDimension()

    property bool fillWidth: !vertical
    property bool fillHeight: vertical

    property QtObject systrayhost: undefined


    onPressed: {
//         print("ST2P MouseEventListener.pressed!")
    }

    onClicked: {
        print("ST2P MouseEventListener.clicked!")
        print("ST2P baseSize: " + root.baseSize);
        togglePopup();
    }

    Timer {
        id: hidePopupTimer
        interval: 10
        running: false
        repeat: false
        onTriggered: {
            print("hidetimer triggered, collapsing " + (root.currentTask == "") )
            if (root.currentTask == "") {
                plasmoid.expanded = false
            }
        }
    }

    function computeDimension() {

        var dim = root.vertical ? compactRepresenation.width : compactRepresenation.height

        var rows = Math.floor(dim / root.itemSize);
        var cols = Math.ceil(systrayhost.shownTasks.length / rows);
        var res = cols * (root.itemSize + root.smallSpacing) + arrow.width;
//         print("DIM itemSize : " + root.itemSize);
//         print("DIM dim : " + dim);
//         print("DIM rows : " + rows);
//         print("DIM cols : " + cols);
//         print("DIM res : " + cols);

        return res;
    }
    function togglePopup() {
        print("toggle popup => " + !plasmoid.expanded);
        if (!plasmoid.expanded) {
            plasmoid.expanded = true
        } else {
            hidePopupTimer.start();
        }
        //plasmoid.expanded = !plasmoid.expanded;
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            print("Empty clicked => " + !plasmoid.expanded)
            plasmoid.expanded = true;
            root.currentTask = "";
            root.expandedItem = null
        }
        //visible: plasmoid.expanded
        onPressed: PlasmaExtras.PressedAnimation { targetItem: arrow }
        onReleased: PlasmaExtras.ReleasedAnimation { targetItem: arrow }
    }

    Rectangle {
        anchors.fill: parent;
        border.width: 1;
        border.color: "black";
        color: "green";
        visible: root.debug;
        opacity: 0.2;
    }

    // Tooltip for arrow --------------------------------
    PlasmaCore.ToolTip {
        id: arrow_tooltip
        target: arrow
        subText: plasmoid.expanded ? i18n("Hide icons") : i18n("Show hidden icons")
    }

    GridView {
        id: gridView
        objectName: "gridView"
        flow: !root.vertical ? GridView.LeftToRight : GridView.TopToBottom

        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            leftMargin: root.vertical ? 0 : root.smallSpacing
            right: arrow.left
        }
        cellWidth: root.itemSize
        cellHeight: cellWidth
/*
        function gridRows() {
            var r = 0;
            if (root.vertical) {
                r = Math.floor(parent.width / root.itemSize);
            } else {
                r = Math.floor(parent.height / root.itemSize);
            }
            print("ST2 ROW: ::::::: " + r);
            return Math.max(1, r);

        }*/
        interactive: false

        model: systrayhost.shownTasks

        delegate: TaskDelegate {}
    }

    PlasmaCore.SvgItem {

        id: arrow

        y: root.itemSize / 4
        anchors {
            leftMargin: root.smallSpacing
            right: parent.right
            //verticalCenter: notificationsContainer.verticalCenter
        }
        width: Math.floor(root.itemSize / 2)
        height: width

        svg: PlasmaCore.Svg { imagePath: "widgets/arrows" }
        elementId: {
            // FIXME : Account for top, bottom, left, right
            if (!vertical) {
                if (plasmoid.expanded) {
                    return "up-arrow";
                } else {
                    return "down-arrow";

                }
            } else {
                if (plasmoid.expanded) {
                    return "left-arrow";
                } else {
                    return "right-arrow";

                }
            }
        }
    }
}
