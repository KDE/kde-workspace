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

    property int minimumWidth: 200
    property int minimumHeight: 200

    property int _h: plasmoid.configuration.itemSize
    property int itemSpacing: 2

    property bool debug: plasmoid.configuration.debug

    property Component compactRepresentation: CompactRepresentation {
        systrayhost: host
    }

    Rectangle {
        anchors.fill: parent;
        border.width: 2;
        border.color: "black";
        color: "blue";
        visible: root.debug;
        opacity: 0.4;
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
    }

    ListView {
        id: hiddenView
        objectName: "hiddenView"

        anchors {
            top: (loadingItem.visible && !plasmoid.expanded) ? loadingItem.bottom : parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        spacing: 4

        model: host.hiddenTasks

        delegate: TaskDelegate {}
    }

}