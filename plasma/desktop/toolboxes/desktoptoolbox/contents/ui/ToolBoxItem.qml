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

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.qtextracomponents 2.0 as QtExtras

Item {
    id: toolBoxItem

    property QtObject proxy: plasmoid.toolBox
    property bool showing: state != "collapsed"
    property int expandedWidth: 240
    property int expandedHeight: 240

    width: childrenRect.width
    height: childrenRect.height
    transformOrigin: {
        if (toolBoxButton.state == "topright") {
            return Item.TopRight;
        } else if (toolBoxButton.state == "right") {
            return Item.Right;
        } else if (toolBoxButton.state == "bottomright") {
            return Item.BottomRight;
        } else if (toolBoxButton.state == "bottom") {
            return Item.Bottom;
        } else if (toolBoxButton.state == "bottomleft") {
            return Item.BottomLeft;
        } else if (toolBoxButton.state == "left") {
            return Item.Left;
        } else if (toolBoxButton.state == "topleft") {
            return Item.TopLeft;
        } else if (toolBoxButton.state == "top") {
            return Item.Top;
        }
    }

    state: "collapsed"

    PlasmaCore.DataSource {
        id: dataEngine
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
    }

    onShowingChanged: {
        print("TB showing changed to " + showing);
        var qmlFile = (!showing) ? "ToolBoxDisappearAnimation.qml" : "ToolBoxAppearAnimation.qml";
        var component = Qt.createComponent(qmlFile);
        if (component.status == Component.Ready) {
            var ani = component.createObject(toolBoxItem);
            ani.targetItem = toolBoxItem;
            ani.start();
        }
    }

    function performOperation(what) {
        var service = dataEngine.serviceForSource("PowerDevil");
        var operation = service.operationDescription(what);
        return service.startOperationCall(operation);
    }

    function lockScreen() {
        print("TB locking...");
        performOperation("lockScreen");
    }

    function lockWidgets(lock) {
        plasmoid.lockWidgets(lock);
    }

    function logout() {
        print("TB shutdown...");
        performOperation("requestShutDown");
    }

    function containmentSettings() {
        plasmoid.action("configure").trigger();
    }

    function shortcutSettings() {
        print("FIXME: implement shortcut settings");
    }

    function showWidgetsExplorer() {
        plasmoid.action("add widgets").trigger();
    }

    function showActivities() {
        print("TB FIXME: Show Activity Manager");
    }


    PlasmaCore.FrameSvgItem {
        id: toolBoxFrame

        width: expandedWidth
        height: actionList.height + toolBoxSvg.topBorder + toolBoxSvg.bottomBorder
        //opacity: toolBoxItem.showing ? 1 : 0

        property Item currentItem: null

        imagePath: "widgets/toolbox"

        Behavior on height {
            NumberAnimation {
                duration: 50
                easing.type: Easing.OutQuad
            }
        }

        Timer {
            id: exitTimer
            interval: 200
            running: true
            repeat: false
            onTriggered: toolBoxHighlight.opacity = 0
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
                    actionIcon: icon
                    objectName: modelData.objectName
                }
            }

            /* // These should come from plugins
            ActionDelegate {
                label: i18n("Activities")
                actionIcon: "preferences-activities"
                objectName: "lock screen"
                onTriggered: showActivities();
            }

            ActionDelegate {
                label: i18n("Shortcut Settings")
                actionIcon: "configure-shortcuts"
                objectName: "shortcut settings"
                onTriggered: containmentSettings();
            }
            */
            ActionDelegate {
                label: i18n("Desktop Settings")
                actionIcon: "configure"
                objectName: "containment settings"
                onTriggered: containmentSettings();
            }

            ActionDelegate {
                label: plasmoid.immutable ? i18n("Unlock Widgets") : i18n("Lock Widgets")
                actionIcon: plasmoid.immutable ? "object-unlocked" : "object-locked"
                objectName: "lock widgets"
                onTriggered: lockWidgets(!plasmoid.immutable);
            }

            ActionDelegate {
                label: i18n("Add Widgets")
                actionIcon: "list-add"
                objectName: "lock screen"
                visible: !plasmoid.immutable
                onTriggered: showWidgetsExplorer();
            }

            ActionDelegate {
                label: i18n("Lock Screen")
                actionIcon: "system-lock-screen"
                objectName: "lock screen"
                onTriggered: lockScreen();
            }

            ActionDelegate {
                label: i18n("Leave")
                actionIcon: "system-shutdown"
                objectName: "leave"
                onTriggered: logout();
            }
        }

        PlasmaComponents.Highlight {
            id: toolBoxHighlight
            opacity: toolBoxFrame.currentItem != null ? 1 : 0
            x: (toolBoxFrame.currentItem != null) ? toolBoxFrame.currentItem.x + toolBoxSvg.topBorder : toolBoxSvg.topBorder
            y: (toolBoxFrame.currentItem != null) ? toolBoxFrame.currentItem.y + toolBoxSvg.leftBorder : toolBoxSvg.leftBorder
            width: actionList.width
            height: (toolBoxFrame.currentItem != null) ? toolBoxFrame.currentItem.height : 0
            Behavior on x {
                NumberAnimation {
                    duration: 150
                    easing.type: Easing.InOutQuad
                }
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: 150
                    easing.type: Easing.InOutQuad
                }
            }
            Behavior on y {
                NumberAnimation {
                    duration: 150
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    states: [
        State {
            name: "expanded"
            PropertyChanges { target: toolBoxFrame; opacity: 1.0; visible: true; }
        },
        State {
            name: "collapsed"
            PropertyChanges { target: toolBoxFrame; opacity: 0; visible: false; }
        }
    ]
}
