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
    width: childrenRect.width
    height: childrenRect.height
    state: "collapsed"
    z: 9999
    property int expandedWidth: 240
    property int expandedHeight: 240
    property int iconSize: 32
    states: [
        State {
            name: "expanded"
            PropertyChanges { target: toolBoxFrame; x: 0 }
            PropertyChanges { target: toolBoxFrame; y: 0 }
        },
        State {
            name: "collapsed"
            PropertyChanges { target: toolBoxFrame; x: 76 }
            PropertyChanges { target: toolBoxFrame; y: -76 }
        }
    ]


    PlasmaCore.FrameSvgItem {
        id: toolBoxFrame
        imagePath: "widgets/translucentbackground"
        //anchors.fill: parent;
//         width: lockedList.width + 32
//         height: lockedlist.width + 32
        width: expandedWidth
        height: expandedHeight
        //enabledBorders: "BottomBorder|LeftBorder"
        opacity: 0
        state: "unlocked" // FIXME: default value
        states: [
            State {
                name: "locked"
                PropertyChanges { target: lockedList; opacity: 1.0; }
                PropertyChanges { target: unlockedList; opacity: 0.0; }
            },
            State {
                name: "unlocked"
                PropertyChanges { target: lockedList; opacity: 0.0; }
                PropertyChanges { target: unlockedList; opacity: 1.0; }
            }
        ]
        Item {
            anchors { fill: parent; leftMargin: 24; topMargin: 24;}
            ListView {
                id: lockedList
                interactive: false
                model: lockedModel
//                 height: contentHeight
//                 width: contentWidth
                anchors { fill: parent; }
    //             spacing: 12
                Rectangle { color: "green"; opacity: 0.4; }
            }

            ListView {
                id: unlockedList
                model: lockedModel
                interactive: false
//                 height: contentHeight
//                 width: contentWidth
                anchors { fill: parent; }
    //             spacing: 12
                Rectangle { color: "blue"; opacity: 0.4; }
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

        VisualItemModel {
            id: lockedModel
            ActionDelegate {
                text: i18n("Activities")
                iconSource: "preferences-activities"
                onTriggered: activitiesAction()
            }
            ActionDelegate {
                text: i18n("Shortcut Settings")
                iconSource: "configure-shortcuts"
                onTriggered: shortcutSettingsAction()
            }
            ActionDelegate {
                text: i18n("Desktop (QML) Settings")
                iconSource: "configure"
                onTriggered: configureAction()
            }
            ActionDelegate {
                text: i18n("Unlock Widgets")
                iconSource: "object-unlocked"
                onTriggered: unlockWidgetsAction()
            }
            ActionDelegate {
                text: i18n("Lock Screen")
                iconSource: "system-lock-screen"
                onTriggered: lockScreenAction()
            }
            ActionDelegate {
                text: i18n("Leave")
                iconSource: "system-shutdown"
                onTriggered: leaveAction()

            }
        }
    }

    QtExtras.QIconItem {
        id: toolBoxButton
        //text: "Add Applet"
        width: iconSize
        height: iconSize
        icon: "plasma"
        anchors { top: parent.top; right: parent.right; }
        //anchors { fill: parent }
        MouseArea {
            anchors.fill: parent
            visible: toolBox.state == "collapsed"
            onClicked: ParallelAnimation {
                ScriptAction {
                    script:toolBox.state = (toolBox.state == "expanded") ? "collapsed" : "expanded";
                }
                PlasmaExtras.AppearAnimation {
                    targetItem: toolBoxFrame
//                     duration: 2000
                }
            }
        }
        MouseArea {
            anchors.fill: parent
            visible: toolBox.state == "expanded"
            onClicked: SequentialAnimation {
                PlasmaExtras.DisappearAnimation {
                    targetItem: toolBoxFrame
//                     duration: 2000
                }
                ScriptAction {
                    script:toolBox.state = (toolBox.state == "expanded") ? "collapsed" : "expanded";
                }
            }
        }
//         onClicked: {
//             var ap = "digital-clock";
//             print("Adding applet..." + ap);
//         }
    }
//     Component.onCompleted: ParallelAnimation {
//         ScriptAction {
//             script:toolBox.state = (toolBox.state == "expanded") ? "collapsed" : "expanded";
//         }
//         PlasmaExtras.AppearAnimation {
//             targetItem: toolBoxFrame
// //                     duration: 2000
//         }
//     }

        function activitiesAction() {
            print("activities action");
        }
        function addWidgetsAction() {
            print("add widgets action");
            plasmoid.action("add widgets")
        }
        function configureAction() {
            print("configure action");
            plasmoid.action("configure")
        }
        function shortcutSettingsAction() {
            print("shortcuts action");
        }
        function unlockWidgetsAction() {
            print("unlock widgets action");
        }
        function lockWidgetsAction() {
            print("lock widgets action");
        }
        function lockScreenAction() {
            print("lock screen");
        }
        function leaveAction() {
            print("leave action");
        }

}
