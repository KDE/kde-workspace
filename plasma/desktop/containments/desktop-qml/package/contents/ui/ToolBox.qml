/***************************************************************************
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.containments 0.1 as PlasmaContainments
import org.kde.qtextracomponents 0.1 as QtExtras

Item {
    id: toolBox

    property QtObject proxy: plasmoid.toolBox
    property int expandedWidth: 240
    property int expandedHeight: 240

    width: childrenRect.width
    height: childrenRect.height
    state: "collapsed"
    z: 9999
    transformOrigin: Item.TopRight

    function performOperation(what) {
        var service = dataEngine.serviceForSource("PowerDevil");
        var operation = service.operationDescription(what);
        return service.startOperationCall(operation);
    }

    function lockScreen() {
        print(" locking...");
        performOperation("lockScreen");
    }

    function logout() {
        print(" shutdown...");
        performOperation("requestShutDown");
    }
    Connections {
        target: proxy
        onShowingChanged: print("Showing now: " + proxy.showing);

    }

    PlasmaCore.DataSource {
        id: dataEngine
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
    }

    PlasmaCore.FrameSvgItem {
        id: toolBoxFrame
        imagePath: "widgets/toolbox"
        width: expandedWidth
        height: actionList.height + toolBoxSvg.topBorder + toolBoxSvg.bottomBorder
//         Rectangle { color: "violet"; opacity: 0.3; anchors.fill: actionList; }
        Timer {
            id: exitTimer
            interval: 100
            running: true
            repeat: false
            //onTriggered: unlockedList.currentIndex = -1
        }
        Column {
            id: actionList
            x: parent.x + toolBoxSvg.topBorder
            y: parent.y + toolBoxSvg.leftBorder
            width: parent.width - (toolBoxSvg.leftBorder + toolBoxSvg.rightBorder)
            Repeater {
                id: unlockedList
                model: proxy.actions
                delegate: ActionDelegate {
                    //height: 36
                    actionIcon: icon
                }
            }
            ActionDelegate {
                label: i18n("Lock Screen")
                actionIcon: "system-lock-screen"
                onTriggered: lockScreen();
            }
            ActionDelegate {
                label: i18n("Leave...")
                actionIcon: "system-shutdown"
                onTriggered: logout();
            }
        }

        /** Action Mapping for ToolBox

        list-add                    Add Panel
        list-add                    Add Widgets
        preferences-activities      Activities                          Activities
        configure-shortcuts         Shortcut Settings                   Shortcut Settings
        configure                   $containment_name Settings          $containment_name Settings
        object-locked               Lock Widgets
        object-unlocked                                                 Unlock Widgets
        system-lock-screen          Lock Screen                         Lock Screen
        system-shutdown             Leave                               Leave

        **/
    }
    states: [
        State {
            name: "expanded"
            PropertyChanges { target: toolBoxFrame; opacity: 1.0 }
        },
        State {
            name: "collapsed"
            PropertyChanges { target: toolBoxFrame; opacity: 0 }
        }
    ]
}
