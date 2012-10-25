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
import org.kde.qtextracomponents 0.1 as QtExtras

Item {
    id: toolBox

    property int expandedWidth: 240
    property int expandedHeight: toolBoxFrame.state == "locked" ? 240 : 320
    //property int expandedHeight: height

    width: childrenRect.width
    height: childrenRect.height
    state: "collapsed"
    z: 9999
    transformOrigin: Item.TopRight
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

    PlasmaCore.FrameSvgItem {
        id: toolBoxFrame
        imagePath: "widgets/toolbox"
        width: expandedWidth
        height: state == "unlocked" ? (addPanelDelegate.height * unlockedModel.count + 24) : (addPanelDelegate.height * lockedModel.count + 24)

        state: "unlocked" // FIXME: correct default value
        states: [
            State {
                name: "locked"
                PropertyChanges { target: unlockedList; model: lockedModel; }
            },
            State {
                name: "unlocked"
                PropertyChanges { target: unlockedList; model: unlockedModel; }
            }
        ]
        ListView {
            id: unlockedList
            model: unlockedModel
            highlight: PlasmaComponents.Highlight {}
            highlightFollowsCurrentItem: true
            interactive: false
            spacing: 0
            Timer {
                id: exitTimer
                interval: 100
                running: false
                repeat: false
                onTriggered: {
                    unlockedList.currentIndex = -1;
                    print("reset list highlight");
                }
            }
            anchors { fill: parent; margins: 12; }
            Component.onCompleted: currentIndex=-1

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

        VisualItemModel {
            id: unlockedModel
            ActionDelegate {
                id: addPanelDelegate
                text: i18n("Add Panel")
                iconSource: "list-add"
                index: 0
                onTriggered: addPanelAction()
            }
            ActionDelegate {
                text: i18n("Add Widgets")
                iconSource: "list-add"
                index: 1
                onTriggered: addWidgetsAction()
            }
            ActionDelegate {
                text: i18n("Activities")
                iconSource: "preferences-activities"
                index: 2
                onTriggered: activitiesAction()
            }
            ActionDelegate {
                text: i18n("Shortcut Settings")
                iconSource: "configure-shortcuts"
                index: 3
                onTriggered: shortcutSettingsAction()
            }
            ActionDelegate {
                text: i18n("Desktop (QML) Settings")
                iconSource: "configure"
//                 onTriggered: configureAction()
                index: 4
                action: plasmoid.action("configure")
            }
            ActionDelegate {
                text: i18n("Lock Widgets")
                iconSource: "object-locked"
                onTriggered: lockWidgetsAction()
                index: 5
                action: plasmoid.action("lock widgets")
            }
            ActionDelegate {
                text: i18n("Lock Screen")
                iconSource: "system-lock-screen"
                index: 6
                onTriggered: lockScreenAction()
            }
            ActionDelegate {
                text: i18n("Leave")
                iconSource: "system-shutdown"
                index: 7
                onTriggered: leaveAction()

            }
        }

        VisualItemModel {
            id: lockedModel
            ActionDelegate {
                text: i18n("Activities")
                iconSource: "preferences-activities"
                index: 0
                onTriggered: activitiesAction()
            }
            ActionDelegate {
                text: i18n("Shortcut Settings")
                iconSource: "configure-shortcuts"
                index: 1
                onTriggered: shortcutSettingsAction()
            }
            ActionDelegate {
                text: i18n("Desktop (QML) Settings")
                iconSource: "configure"
                index: 2
                //onTriggered: configureAction()
                action: plasmoid.action("configure")
            }
            ActionDelegate {
                text: i18n("Unlock Widgets")
                iconSource: "object-unlocked"
                index: 3
                onTriggered: unlockWidgetsAction()
                action: plasmoid.action("Unlock widgets")
            }
            ActionDelegate {
                text: i18n("Lock Screen")
                iconSource: "system-lock-screen"
                index: 4
                onTriggered: lockScreenAction()
            }
            ActionDelegate {
                text: i18n("Leave")
                iconSource: "system-shutdown"
                index: 5
                onTriggered: leaveAction()
            }
        }
    }

    function activitiesAction() {
        print("activities action");
    }
    function addPanelAction() {
        print("add panel action ??");
        plasmoid.action("add panel") // ??
    }
    function addWidgetsAction() {
        print("add widgets action");
        plasmoid.action("add widgets");
    }
    function configureAction() {
        print("configure action");
        var ac = plasmoid.action("configure");
        ac.trigger();
    }
    function shortcutSettingsAction() {
        print("shortcuts action ??");
        plasmoid.action("configure shortcuts"); // ??
    }
    function unlockWidgetsAction() {
        print("unlock widgets action");
        toolBoxFrame.state = "unlocked";
    }
    function lockWidgetsAction() {
        print("lock widgets action");
        toolBoxFrame.state = "locked";
    }
    function lockScreenAction() {
        print("lock screen ??");
    }
    function leaveAction() {
        print("leave action ??");
    }

}
