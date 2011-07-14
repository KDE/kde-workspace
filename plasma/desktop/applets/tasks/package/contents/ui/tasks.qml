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
    property bool kcfg_showOnlyCurrentDesktop: true
    property bool kcfg_showOnlyCurrentActivity: false
    property bool kcfg_showOnlyMinimized: false

    Component.onCompleted: {
        plasmoid.addEventListener ('ConfigChanged', configChanged);
        configChanged()
    }

    function configChanged() {
        kcfg_fillRows = plasmoid.readConfig("fillRows");
        kcfg_showTooltip = plasmoid.readConfig("showTooltip");
        kcfg_highlightWindiws = plasmoid.readConfig("highlightWindows");
        kcfg_maxRows = plasmoid.readConfig("maxRows");
        kcfg_groupingStrategy = plasmoid.readConfig("groupingStrategy");
        kcfg_groupWhenFull = plasmoid.readConfig("groupWhenFull");
        kcfg_sortingStrategy = plasmoid.readConfig("sortingStrategy");
        kcfg_showOnlyCurrentScreen = plasmoid.readConfig("showOnlyCurrentScreen");
        kcfg_showOnlyCurrentDesktop = plasmoid.readConfig("showOnlyCurrentDesktop");
        kcfg_showOnlyCurrentActivity = plasmoid.readConfig("showOnlyCurrentActivity");
        kcfg_showOnlyMinimized = plasmoid.readConfig("showOnlyMinimized");
    }

    PlasmaCore.DataSource {
        id: tasksSource
        engine: "tasks"
        connectedSources: sources
        interval: 0
    }

    ListView {
        id: tasksView
        anchors.fill: parent
        orientation: Qt.Horizontal
        model: tasksSource.sources
        delegate: taskItem
    }

    Component {
        id: taskItem

        TaskItem {
            icon: tasksSource.data[modelData]["icon"]
            label: tasksSource.data[modelData]["name"]
            focused: tasksSource.data[modelData]["active"]
            width: tasks.width/tasksView.model.length
            height: tasks.height

            onFocus: {
                minimized = tasksSource.data[modelData]["minimized"];
                service = tasksSource.serviceForSource (modelData);
                operation = service.operationDescription ("toggleMinimized");
                job = service.startOperationCall (operation);
                if ( minimized ) {
                    operation = service.operationDescription ("activate");
                    service.startOperationCall (operation);
                }
            }
        }
    }
}
