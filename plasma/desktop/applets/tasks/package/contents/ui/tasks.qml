/*
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: tasks

    property bool kcfg_fillRows: false
    property bool kcfg_showTooltip: true
    property bool kcfg_highlightWindows: false
    property int kcfg_maxRows: 2
    property int kcfg_groupingStrategy: 2
    property bool kcfg_groupWhenFull: true
    property int kcfg_sortingStrategy: 0
    property bool kcfg_showOnlyCurrentScreen: false
    property bool kcfg_showOnlyCurrentDesktop: false
    property bool kcfg_showOnlyCurrentActivity: true
    property bool kcfg_showOnlyMinimized: false
    property int count: 0

    Component.onCompleted: {
        plasmoid.addEventListener ('ConfigChanged', configChanged);
        //tasksView.model = [];
    }

    function configChanged() {
        kcfg_fillRows = plasmoid.readConfig("fillRows");
        kcfg_showTooltip = plasmoid.readConfig("showTooltip");
        kcfg_highlightWindows = plasmoid.readConfig("highlightWindows");
        kcfg_maxRows = plasmoid.readConfig("maxRows");
        kcfg_groupingStrategy = plasmoid.readConfig("groupingStrategy");
        kcfg_groupWhenFull = plasmoid.readConfig("groupWhenFull");
        kcfg_sortingStrategy = plasmoid.readConfig("sortingStrategy");
        kcfg_showOnlyCurrentScreen = plasmoid.readConfig("showOnlyCurrentScreen");
        kcfg_showOnlyCurrentDesktop = plasmoid.readConfig("showOnlyCurrentDesktop");
        kcfg_showOnlyCurrentActivity = plasmoid.readConfig("showOnlyCurrentActivity");
        kcfg_showOnlyMinimized = plasmoid.readConfig("showOnlyMinimized");
        reload();
    }

    PlasmaCore.DataSource {
        id: tasksSource
        engine: "tasks"
        connectedSources: sources
        interval: 0
        onSourcesChanged: reload()
        Component.onCompleted: reload()
    }

    function reload()
    {
        print("==reload==");
        var sources = tasksSource.sources;
        var mymodel = new Array();
        for (i=0; i<sources.length; i++) {
            item_id = sources[i];
            print ("===test "+item_id);
            visible = true;
            if (kcfg_showOnlyCurrentActivity)
                visible = tasksSource.data[item_id]["onCurrentActivity"] || tasksSource.data[item_id]["onAllActivities"];
            if (visible && kcfg_showOnlyCurrentDesktop)
                visible = tasksSource.data[item_id]["onCurrentDesktop"] || tasksSource.data[item_id]["onAllDesktops"];
            if (visible && kcfg_showOnlyMinimized)
                visible = tasksSource.data[item_id]["minimized"];
            if (visible)
            {
                print ("===show "+item_id);
                mymodel.push (item_id);
            }
            else print ("===noshow "+item_id);
        }
        tasksView.model = mymodel;
    }

    ListView {
        id: tasksView
        anchors.fill: parent
        orientation: Qt.Horizontal
        delegate: taskItem
    }

    Component {
        id: taskItem

        TaskItem {
            item_id: modelData
            icon: tasksSource.data[modelData]["icon"]
            label: tasksSource.data[modelData]["name"]
            focused: tasksSource.data[modelData]["active"]
            width: Math.min(tasks.width/tasksView.count, 270)
            height: tasks.height
            showLabel: width >= 85

            onClicked: {
                current = tasksSource.data[modelData]["onCurrentDesktop"];
                service = tasksSource.serviceForSource (modelData);
                if ( focused ) {
                    operation = service.operationDescription ("toggleMinimized");
                    service.startOperationCall (operation);
                }
                else {
                    operation = service.operationDescription ("activate");
                    service.startOperationCall (operation);
                }
            }
        }
    }
}
