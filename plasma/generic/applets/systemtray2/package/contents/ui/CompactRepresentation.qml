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

import org.kde.private.systemtray 2.0 as SystemTray


Item {
    id: compactRepresenation

    // TODO: vertical formfactor
    //property int minimumWidth: (1.5 + systrayhost.shownTasks.length) * (plasmoid.configuration.itemSize + itemSpacing) + (2 * itemSpacing)
    property int minimumWidth: computeDimension()
    property int maximumWidth: minimumWidth

    property bool fillWidth: !vertical
    property bool fillHeight: vertical

    property QtObject systrayhost: undefined

    function computeDimension() {
        var dim = 0;

        // systray tasks + notifications widget
        var x = vertical ? compactRepresenation.width : compactRepresenation.height
        dim = (systrayhost.shownTasks.length + 1) * (x + itemSpacing);
        dim = dim + arrow.width + itemSpacing + itemSpacing;

        return dim;
    }

    function loadNotificationsPlasmoid() {
        var plugin = "org.kde.notifications";
        systrayhost.rootItem = gridView;
        var notificationsPlasmoid = systrayhost.notificationsPlasmoid(plugin);
        if (notificationsPlasmoid == null) {
            print("Bah. Failed to load " + plugin);
            return;
        }
        notificationsPlasmoid.parent = notificationsContainer;
        notificationsPlasmoid.anchors.fill = notificationsContainer;
    }

    function togglePopup() {
        plasmoid.expanded = !plasmoid.expanded;
    }

    MouseArea {
        anchors.fill: parent
        onClicked: togglePopup()
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
            leftMargin: root.vertical ? 0 : itemSpacing
            right: arrow.left
        }
        cellWidth: root.vertical ? parent.width / gridRows() : parent.height / gridRows()

        function gridRows() {
            var r = 0;
            if (root.vertical) {
                r = Math.floor(parent.width / plasmoid.configuration.itemSize);
            } else {
                r = Math.floor(parent.height / plasmoid.configuration.itemSize);
            }
            print("ST2 ROW: ::::::: " + r);
            return Math.max(1, r);

        }
        cellHeight: cellWidth
        interactive: false

        model: systrayhost.shownTasks

        delegate: TaskDelegate {}
    }

    PlasmaCore.SvgItem {

        id: arrow

        anchors {
            leftMargin: itemSpacing
            right: parent.right
            verticalCenter: notificationsContainer.verticalCenter
        }
        width: _h / 2
        height: width

        svg: PlasmaCore.Svg { imagePath: "widgets/arrows" }
        elementId: "up-arrow"
    }
}
