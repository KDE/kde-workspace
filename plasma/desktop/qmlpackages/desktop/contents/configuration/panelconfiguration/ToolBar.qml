/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0

Item {
    id: root
    state: parent.state
    implicitWidth: row.width + 20
    implicitHeight: row.height + 20
    GridLayout {
        id: row
        columns: dialogRoot.vertical ? 1 : 2
        rows: dialogRoot.vertical ? 2 : 1
        anchors.centerIn: parent

        rowSpacing: 0
        columnSpacing: 0

        EdgeHandle {}
        SizeHandle {}
    }

    GridLayout {
        id: buttonsLayout
        rows: 1
        columns: 1
        flow:plasmoid.formFactor == PlasmaCore.Types.Horizontal ? GridLayout.TopToBottom : GridLayout.LeftToRight

        anchors.margins: rowSpacing

        PlasmaComponents.Button {
            text: i18n("Add Widgets...")
            iconSource: "list-add"
            onClicked: {
                configDialog.showAddWidgetDialog();
            }
        }

//TODO
//         PlasmaComponents.Button {
//             iconSource: "distribute-horizontal-x"
//             text: i18n("Add spacer")
//             onClicked: {
//             }
//         }

        PlasmaComponents.Button {
            id: settingsButton
            iconSource: "configure"
            text: i18n("More Settings...")
            property var contextMenu
            onClicked: {
                if (!contextMenu) {
                    contextMenu = contextMenuComponent.createObject(buttonsLayout)
                }
                contextMenu.open()
            }
        }

        PlasmaComponents.ToolButton {
            iconSource: "window-close"
            onClicked: {
                configDialog.close()
            }
        }

        Component {
            id: contextMenuComponent
            PlasmaComponents.ContextMenu {
                visualParent: settingsButton
                PlasmaComponents.MenuItem {
                    id: leftToggle
                    text: i18n("Left")
                    checkable: true
                    checked: panel.alignment == Qt.AlignLeft
                    onClicked: panel.alignment = Qt.AlignLeft
                    onToggled: {
                        if (checked) {
                            centerToggle.checked = false
                            rightToggle.checked = false
                        }
                    }
                }
                PlasmaComponents.MenuItem {
                    id: centerToggle
                    text: i18n("Center")
                    checkable: true
                    checked: panel.alignment == Qt.AlignCenter
                    onClicked: panel.alignment = Qt.AlignCenter
                    onToggled: {
                        if (checked) {
                            leftToggle.checked = false
                            rightToggle.checked = false
                        }
                    }
                }
                PlasmaComponents.MenuItem {
                    id: rightToggle
                    text: i18n("Right")
                    checkable: true
                    checked: panel.alignment == Qt.AlignRight
                    onClicked: panel.alignment = Qt.AlignRight
                    onToggled: {
                        if (checked) {
                            centerToggle.checked = false
                            leftToggle.checked = false
                        }
                    }
                }
            }
        }

    }
//BEGIN States
    states: [
        State {
            name: "TopEdge"
            PropertyChanges {
                target: root
                height: root.implicitHeight
            }
            AnchorChanges {
                target: root
                anchors {
                    top: undefined
                    bottom: root.parent.bottom
                    left: root.parent.left
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: buttonsLayout
                anchors {
                    verticalCenter: root.verticalCenter
                    horizontalCenter: undefined
                    top: undefined
                    bottom: undefined
                    left: undefined
                    right: root.right
                }
            }
        },
        State {
            name: "BottomEdge"
            PropertyChanges {
                target: root
                height: root.implicitHeight
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: undefined
                    left: root.parent.left
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: buttonsLayout
                anchors {
                    verticalCenter: root.verticalCenter
                    horizontalCenter: undefined
                    top: undefined
                    bottom: undefined
                    left: undefined
                    right: root.right
                }
            }
        },
        State {
            name: "LeftEdge"
            PropertyChanges {
                target: root
                width: root.implicitWidth
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: root.parent.bottom
                    left: undefined
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: buttonsLayout
                anchors {
                    verticalCenter: undefined
                    horizontalCenter: root.verticalCenter
                    top: undefined
                    bottom: root.bottom
                    left: undefined
                    right: undefined
                }
            }
        },
        State {
            name: "RightEdge"
            PropertyChanges {
                target: root
                width: root.implicitWidth
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: root.parent.bottom
                    left: root.parent.left
                    right: undefined
                }
            }
            AnchorChanges {
                target: buttonsLayout
                anchors {
                    verticalCenter: undefined
                    horizontalCenter: root.verticalCenter
                    top: undefined
                    bottom: root.bottom
                    left: undefined
                    right: undefined
                }
            }
        }
    ]
//END States
}
