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

    function checkTask(task) {
        if (task.taskItemExpanded == null) return;
        //if (root.expandedItem == null) return;

//         print("ST2P Task name " + task.name);
        //print("ST2P Task !null " + (task.taskItemExpanded != null));
        //print("ST2P Task !undefined " + (task.taskItemExpanded != undefined));
        //print("ST2P Task   id " + task.taskId);
        var isthis = (task.taskId == root.currentTask);

//        print("ST2P      samesame?? " + isthis);
        //print("ST2P Task expanded?? " + task.expanded);
        print("ST2P Checking Task name " + task.taskId + " this? " + isthis);

        if (!isthis) {
            print("             ----> ST2P !!!!!!!!! Collapsing ?? " + task.taskId);
            task.expanded = false;
            //task.taskItemExpanded.opacity = 0.1
        } else {
            //task.taskItemExpanded.opacity = 1
        }
    }
    function clearExpanded() {
        var _hidden = host.hiddenTasks;
        for (var i = 0; i < _hidden.length; i++) {
            checkTask(_hidden[i]);
        }
        var _shown = host.shownTasks;
        for (i = 0; i < _shown.length; i++) {
            checkTask(_shown[i]);

        }
    }
    Connections {
        target: root
        onCurrentTaskChanged: print("Current Task now: " + root.currentTask);
        onExpandedItemChanged: {

            print("ST2P main.qml =======================================");
            print("ST2P main.qml Expanded item changed ... reparenting " + root.currentTask);

//             if (root.expandedItem == null && plasmoid.expanded) {
//                 //plasmoid.expanded = false;
//                 //clearExpanded();
//                 expandedItemContainer.clear();
//             }

            if (root.expandedItem != null) {
                print("not null");

                root.expandedItem.parent = expandedItemContainer;
                root.expandedItem.anchors.fill = expandedItemContainer;
                //expandedItemContainer.currentPage.opacity = 0.2
                expandedItemContainer.replace(root.expandedItem);
                //expandedItemContainer.currentPage.opacity = 1
                if (expandedItemContainer.currentPage != root.expandedItem) {
                    //expandedItemContainer.replace(root.expandedItem);
                } else {
                    //expandedItemContainer.replace(root.expandedItem);
                    //hiddenView.width = parent.width;
                    //clearExpanded();
                    //expandedItemContainer.clear();
                }
                //if (!plasmoid.expanded) {
//                    hiddenView.width = _h * 2;
                    //plasmoid.expanded = true;
                //}
            } else {
                print("null");
    //             if (plasmoid.expanded) {
    //                 plasmoid.expanded = false;
    //             }
                print("ST2P main.qml Resetting");
                //hiddenView.width = parent.width;
                if (expandedItemContainer.currentPage != null) {
                    //plasmoid.expanded = false;
                    expandedItemContainer.clear();
                }
                root.currentTask = "";
            }
            clearExpanded();
        }

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

    MouseArea {
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: expandedItemContainer.left
        }
        onClicked: {
            print("ST2 Clearing on request");
            clearExpanded();
            expandedItemContainer.clear();
            root.currentTask = ""
            root.expandedItem = null;
            //plasmoid.expanded = true;
        }
//         Rectangle { anchors.fill: parent; color: "orange"; opacity: 1; }

    }

    ListView {
        id: hiddenView
        objectName: "hiddenView"
        clip: true
        interactive: (contentHeight > height)
        width: parent.width

        anchors {
            top: (loadingItem.visible && !plasmoid.expanded) ? loadingItem.bottom : parent.top
            bottom: (loadingItem.visible && !plasmoid.expanded) ? undefined : parent.bottom
            left: parent.left
            //right: parent.right
        }
        spacing: 4

        model: host.hiddenTasks

        delegate: TaskListDelegate {}
    }

    PlasmaCore.SvgItem {
        id: separator

        width: lineSvg.elementSize("vertical-line").width;
        height: parent.width;
        visible: root.expandedItem != null

        anchors {
            right: expandedItemContainer.left;
            top: parent.right;
            bottom: parent.bototm;
            rightMargin: root.smallSpacing;
        }
        elementId: "vertical-line";

        svg: PlasmaCore.Svg {
            id: lineSvg;
            imagePath: "widgets/line";
        }
    }

    Connections {
        target: plasmoid
        onExpandedChanged: {
            if (!plasmoid.expanded) {
                root.expandedItem = null;
                expandedItemContainer.clear();
                root.currentTask = "";
            }
            clearExpanded();
        }
    }

    PlasmaComponents.PageStack {
        id: expandedItemContainer
        anchors {
            left: parent.left
            leftMargin: root.itemSize + root.largeSpacing * 2
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
    }
}