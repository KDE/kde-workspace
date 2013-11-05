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
    id: root
    objectName: "SystemTrayRootItem"

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    property int minimumWidth: 200 // just needs to run out of space in the panel ...
    property int minimumHeight: 200 // ... but not too big to screw up initial layouts

    property int _h: plasmoid.configuration.itemSize
    property int itemSpacing: 2

    property bool debug: plasmoid.configuration.debug

    property Item expandedItem: undefined

    property Component compactRepresentation: CompactRepresentation {
        systrayhost: host
    }

    onExpandedItemChanged: {
        print("ST2P main.qml Expanded item changed ... reparenting ");

        if (expandedItem == null && plasmoid.expanded) {
            //plasmoid.expanded = false;
            expandedItemContainer.clear();
        }

        if (expandedItem != null) {
            if (expandedItemContainer.currentPage != expandedItem) {
                expandedItemContainer.replace(expandedItem);
            } else {
                hiddenView.width = parent.width;
                expandedItemContainer.clear();
            }
            if (!plasmoid.expanded) {
                hiddenView.width = _h * 2;
                plasmoid.expanded = true;
            }
        } else {
//             if (plasmoid.expanded) {
//                 plasmoid.expanded = false;
//             }
            print("ST2P main.qml Resetting");
            hiddenView.width = parent.width;
            if (expandedItemContainer.currentPage == null) {
                plasmoid.expanded = false;
            }
            expandedItemContainer.clear();
        }
    }

    Rectangle {
        anchors.fill: parent;
        border.width: 2;
        border.color: "black";
        color: "blue";
        visible: root.debug;
        opacity: 0.2;
    }
    SystemTray.Host {
        id: host
        rootItem: hiddenView
    }

    /* This mechanism hides inactive items until the layout figures out that it has to
     * collapse the Plasmoid.
     *
     * It also takes care of delaying the loading of hidden items (they're hidden, and
     * can be loaded later as to not block other, visible components from loading). It
     * does so by simply pushing them out of the viewport, which means the ListView won't
     * render the TaskDelegates until then.
     */
    Item {
        id: loadingItem
        anchors.fill: parent
        anchors.margins: height
        Timer {
            running: true
            interval: 4000
            onTriggered: loadingItem.visible = false
        }
    }

    ListView {
        id: hiddenView
        objectName: "hiddenView"
        clip: true

        width: parent.width

        anchors {
            top: (loadingItem.visible && !plasmoid.expanded) ? loadingItem.bottom : parent.top
            bottom: (loadingItem.visible && !plasmoid.expanded) ? undefined : parent.bottom
            left: parent.left
            //right: parent.right
        }
        spacing: 4

        model: host.hiddenTasks

        delegate: TaskListDelegate {
            expanded: (root.expandedItem == null)
        }
    }

    Connections {
        target: plasmoid
        onExpandedChanged: {
            if (!plasmoid.expanded) {
                //root.expandedItem = undefined;
                //expandedItemContainer.clear();
            }
        }
    }

    PlasmaComponents.PageStack {
        id: expandedItemContainer
        anchors {
            left: parent.left
            leftMargin: _h * 1.2
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
    }

}