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

    property int minimumWidth: !root.vertical ? computeDimension() : computeDimensionHeight()
    property int minimumHeight: root.vertical ? computeDimension() : computeDimensionHeight()
    property int maximumWidth: minimumWidth
    property int maximumHeight: minimumHeight

    property bool fillWidth: false
    property bool fillHeight: false

    property QtObject systrayhost: undefined

    onClicked: {
        print("ST2P MouseEventListener.clicked!")
        //togglePopup(); // FIXME reenable;
    }
    onPressed: PlasmaExtras.PressedAnimation { targetItem: arrow }
    onReleased: PlasmaExtras.ReleasedAnimation { targetItem: arrow }

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
        var res = cols * (root.itemSize + theme.smallSpacing*4) + theme.smallSpacing + arrow.width;
        print(" computeDimension()  " + res);
        return res;
    }

    function computeDimensionHeight() {
        var dim = root.vertical ? compactRepresenation.width : compactRepresenation.height
        var rows = Math.floor(dim / root.itemSize);
        //var cols = Math.ceil(systrayhost.shownTasks.length / rows);
        var rr = rows * (root.itemSize + theme.smallSpacing);
        //var res = cols * (root.itemSize + root.smallSpacing) + arrow.width;
        print(" computeDimensionHeight()  " + rr);
        return rr;
    }

    function togglePopup() {
        print("toggle popup => " + !plasmoid.expanded);
        if (!plasmoid.expanded) {
            plasmoid.expanded = true
        } else {
            hidePopupTimer.start();
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            print("Empty clicked => " + !plasmoid.expanded)
            plasmoid.expanded = true;
            root.currentTask = "";
            root.expandedItem = null
            print("!! compactRepresenation w/h : " + compactRepresenation.width + " / " + compactRepresenation.height);
            print("!! compactRepresenation: x/y" + compactRepresenation.x + " / " + compactRepresenation.y);
        }
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

    Component {
        id: taskDelegateComponent
        TaskDelegate {
            id: taskDelegate
            //task: ListView.view.model
        }
    }

    GridView {
        id: gridView
        objectName: "gridView"
        flow: !root.vertical ? GridView.LeftToRight : GridView.TopToBottom

//         y: (parent.height - root.itemSize) / 2
        anchors {
            top: parent.top
            bottom: parent.bottom
            topMargin: !root.vertical ? ((parent.height - root.itemSize) / 2) - 1 : theme.smallSpacing
//             verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: root.vertical ? 0 : root.smallSpacing
            right: arrow.left
        }
        cellWidth: !root.vertical ? root.itemSize + theme.smallSpacing * 4 : root.itemSize
        cellHeight: root.vertical ? root.itemSize + theme.smallSpacing * 4 : root.itemSize
        interactive: false

        model: systrayhost.shownTasks

        delegate: taskDelegateComponent
    }

    PlasmaCore.SvgItem {

        id: arrow

        y: root.itemSize / 4
        anchors {
            leftMargin: root.smallSpacing
            right: parent.right
        }
        width: Math.floor(root.itemSize / 2)
        height: width

        svg: PlasmaCore.Svg { imagePath: "widgets/arrows" }
        elementId: {

            var exp = plasmoid.expanded; // flip for bottom edge and right edge

            if (plasmoid.location == PlasmaCore.Types.BottomEdge) {
		return (exp) ? "down-arrow" : "up-arrow"
	    } else if (plasmoid.location == PlasmaCore.Types.TopEdge) {
		return (exp) ? "up-arrow" : "down-arrow"
	    } else if (plasmoid.location == PlasmaCore.Types.LeftEdge) {
		return (exp) ? "left-arrow" : "right-arrow"
	    } else {
		return (exp) ? "right-arrow" : "left-arrow"
	    }

        }
    }

    onHeightChanged: ttt.running = true
    onWidthChanged: ttt.running = true

    Timer {
        id: ttt
        interval: 50
        running: false
        repeat: false
        onTriggered: {
            var dim = root.vertical ? compactRepresenation.width : compactRepresenation.height;
            root.preferredItemSize = dim - root.smallSpacing;
        }
    }
}
