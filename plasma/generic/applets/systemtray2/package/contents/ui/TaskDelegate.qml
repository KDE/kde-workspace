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
    id: taskItemContainer
    objectName: "taskItemContainer"

    width: gridView.cellWidth
    height: gridView.cellHeight
    property int taskStatus: status

    Rectangle {
        anchors.fill: parent;
        border.width: 1;
        border.color: "violet";
        color: "pink";
        visible: root.debug;
        opacity: 0.5;
    }

    PlasmaCore.IconItem {
        id: itemIcon
        width: parent.height
        height: parent.height
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
        //visible: source != ""
        source: iconName != "" ? iconName : (typeof(icon) != "undefined" ? icon : "")
    }

    PulseAnimation {
        targetItem: taskItemContainer
        running: status == SystemTray.Task.NeedsAttention
    }

    onTaskStatusChanged: {
        print("ST2 status changed to " + taskStatusString());
    }
    // just for debugging purposes
    function taskStatusMnemonic() {
        if (status == SystemTray.Task.Passive) {
            return "--";
        } else if (status == SystemTray.Task.Active) {
            return "o";
        } else if (status == SystemTray.Task.NeedsAttention) {
            return "\o/";
        }
        return "??";
    }
    function taskStatusString() {
        if (status == SystemTray.Task.Passive) {
            return "Passive";
        } else if (status == SystemTray.Task.Active) {
            return "Active";
        } else if (status == SystemTray.Task.NeedsAttention) {
            return "NeedsAttention";
        }
        return "Unknown";
    }

    Component.onCompleted: {
        //host.rootItem = gridView;
        print(" ST2 taskitem created: " + taskItem + " " + iconName);
        if ((taskItem != undefined)) {
            //print( " TASK ITEM CHANGED"  + (taskItem != undefined));
            taskItem.parent = taskItemContainer;
            taskItem.anchors.fill = taskItem.parent;
        }
    }
}
