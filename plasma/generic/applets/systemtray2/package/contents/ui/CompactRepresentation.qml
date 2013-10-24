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

    property QtObject systrayhost: undefined

    function loadNotificationsPlasmoid() {
        var plugin = "org.kde.systrayplasmoidtest";
        plugin = "org.kde.notifications";
        print("Loading notifications plasmoid: " + plugin);
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

    // Tooltip for arrow --------------------------------
    PlasmaCore.ToolTip {
        id: arrow_tooltip
        target: arrow
        subText: plasmoid.expanded ? i18n("Hide icons") : i18n("Show hidden icons")
    }

    Item {
        id: notificationsContainer

        anchors {
            top: parent.top
            left: parent.left
        }
        height: _h
        width: _h
        /*
        Rectangle {
            anchors.fill: parent;
            border.width: 2;
            border.color: "black";
            color: "transparent";
            opacity: 0;
        }
        */
        Timer {
            interval: 0
            running: true
            onTriggered: {
                loadNotificationsPlasmoid();
            }
        }
    }

    GridView {
        id: gridView
        objectName: "gridView"

        anchors {
            top: notificationsContainer.top
            bottom: parent.bottom
            left: notificationsContainer.right
            leftMargin: itemSpacing
            right: arrow.left
        }
        cellWidth: _h + itemSpacing
        cellHeight: _h + itemSpacing
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
