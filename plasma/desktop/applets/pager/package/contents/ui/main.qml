/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.qtextracomponents 2.0 as QtExtraComponents
import org.kde.draganddrop 2.0
import org.kde.plasma.private.pager 2.0
import "utils.js" as Utils

Item {
    id: root

    property int minimumWidth
    property int minimumHeight

    property bool dragging: false
    property int dragId

    property int dragSwitchDesktopId: -1

    anchors.fill: parent
    visible: repeater.count > 1

    property color windowActiveOnActiveDesktopColor: theme.textColor
    property color windowInactiveOnActiveDesktopColor: theme.textColor
    property color windowActiveColor: theme.textColor
    property color windowActiveBorderColor: theme.textColor
    property color windowInactiveColor: theme.textColor
    property color windowInactiveBorderColor: theme.textColor

    Connections {
        target: theme
        onThemeChanged: {
            windowActiveOnActiveDesktopColor = theme.textColor
            windowActiveOnActiveDesktopColor.a = 0.6
            windowActiveColor = theme.textColor
            windowActiveColor.a = 0.5;
            windowActiveBorderColor = theme.textColor
            //windowActiveBorderColor = 1

            windowInactiveOnActiveDesktopColor = theme.textColor
            windowInactiveOnActiveDesktopColor.a = 0.35;
            windowInactiveColor = theme.textColor
            windowInactiveColor.a = 0.17;
            windowInactiveBorderColor = theme.textColor
            windowInactiveBorderColor.a = 0.5
        }
    }
    Component.onCompleted: {
            windowActiveOnActiveDesktopColor = theme.textColor
            windowActiveOnActiveDesktopColor.a = 0.6
            windowActiveColor = theme.textColor
            windowActiveColor.a = 0.5;
            windowActiveBorderColor = theme.textColor
            //windowActiveBorderColor = 1

            windowInactiveOnActiveDesktopColor = theme.textColor
            windowInactiveOnActiveDesktopColor.a = 0.35;
            windowInactiveColor = theme.textColor
            windowInactiveColor.a = 0.17;
            windowInactiveBorderColor = theme.textColor
            windowInactiveBorderColor.a = 0.5
    }

    Pager {
        id: pager
        orientation: plasmoid.formFactor == PlasmaCore.Types.Vertical ? Qt.Vertical : Qt.Horizontal
        size: Qt.size(root.width, root.height)
        showWindowIcons: plasmoid.configuration.showWindowIcons
        currentDesktopSelected: plasmoid.configuration.currentDesktopSelected
        displayedText: plasmoid.configuration.displayedText
    }

    Timer {
        id: dragTimer
        interval: 1000
        onTriggered: {
            if (dragSwitchDesktopId != -1 && dragSwitchDesktopId !== pager.currentDesktop-1) {
                pager.changeDesktop(dragSwitchDesktopId);
            }
        }
    }

    Repeater {
        id: repeater
        model: pager.model

        Item {
            id: desktop

            property int desktopId: index
            property string desktopName: model.desktopName ? model.desktopName : ""
            property bool active: (desktopId === pager.currentDesktop-1)

            x: model.x
            y: model.y
            width: model.width
            height: model.height

            PlasmaCore.FrameSvgItem {
                anchors.fill: parent
                z: 1 // to make sure that the FrameSvg will be placed on top of the windows
                imagePath: "widgets/pager"
                prefix: (desktopMouseArea.enabled && desktopMouseArea.containsMouse) || (root.dragging && root.dragId == desktopId) ?
                            "hover" : (desktop.active ? "active" : "normal")

                onPrefixChanged: {
                    if (prefix == "hover")
                        pager.updateToolTip(desktopId);
                }
            }

            DropArea {
                id: droparea
                anchors.fill: parent
                onDragEnter: {
                    root.dragSwitchDesktopId = desktop.desktopId;
                    dragTimer.start();
                }
                onDragLeave: {
                    root.dragSwitchDesktopId = -1;
                    dragTimer.stop();
                }
                onDrop: {
                    pager.dropMimeData(event.mimeData, desktop.desktopId);
                    root.dragSwitchDesktopId = -1;
                    dragTimer.stop();
                }
            }

            MouseArea {
                id: desktopMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: pager.changeDesktop(desktopId);
            }

            Item {
                id: clipRect
                x: 1
                y: 1
                width: desktop.width - 2
                height: desktop.height - 2
                clip: true

                PlasmaComponents.Label {
                    id: desktopText
                    anchors.centerIn: parent
                    text: pager.displayedText == Pager.Name ? desktop.desktopName
                                                : (pager.displayedText == Pager.Number ? desktop.desktopId+1 : "")
                    
                }

                Repeater {
                    model: windows

                    Rectangle {
                        id: windowRect

                        property int windowId: model.windowId

                        /* since we move clipRect with 1, move it back */
                        x: model.x - 1
                        y: model.y - 1
                        width: model.width
                        height: model.height
                        color: {
                            if (desktop.active) {
                                if (model.active)
                                    return windowActiveOnActiveDesktopColor;
                                else
                                    return windowInactiveOnActiveDesktopColor;
                            } else {
                                if (model.active)
                                    return windowActiveColor;
                                else
                                    return windowInactiveColor;
                            }
                        }

                        border.width: 1
                        border.color: model.active ? windowActiveBorderColor
                                                   : windowInactiveBorderColor

                        QtExtraComponents.QPixmapItem {
                            id: icon
                            anchors.centerIn: parent
                            pixmap: model.icon
                            height: nativeHeight
                            width: nativeWidth
                            visible: pager.showWindowIcons && (windowRect.width >= icon.width) && (windowRect.height >= icon.height)
                        }

                        MouseArea {
                            id: windowMouseArea
                            anchors.fill: parent
                            drag.target: windowRect
                            drag.axis: Drag.XandYAxis
                            drag.minimumX: -windowRect.width/2
                            drag.maximumX: root.width - windowRect.width/2
                            drag.minimumY: -windowRect.height/2
                            drag.maximumY: root.height - windowRect.height/2

                            // used to save the state of some properties before the dragging
                            QtObject {
                                id: saveState
                                property int x: -1
                                property int y: -1
                                property variant parent
                                property int desktop: -1
                                property int mouseX: -1
                                property int mouseY: -1
                            }

                            drag.onActiveChanged: {
                                root.dragging = drag.active;
                                desktopMouseArea.enabled = !drag.active;
                            }

                            // reparent windowRect to enable the dragging for other desktops
                            onPressed: {
                                if (windowRect.parent == root)
                                    return;

                                saveState.x = windowRect.x;
                                saveState.y = windowRect.y
                                saveState.parent = windowRect.parent;
                                saveState.desktop = desktop.desktopId;
                                saveState.mouseX = mouseX;
                                saveState.mouseY = mouseY;

                                var value = root.mapFromItem(clipRect, windowRect.x, windowRect.y);
                                windowRect.x = value.x;
                                windowRect.y = value.y
                                windowRect.parent = root;
                            }

                            onReleased: {
                                if (root.dragging) {
                                    pager.moveWindow(windowRect.windowId, windowRect.x, windowRect.y,
                                                     root.dragId, saveState.desktop);
                                } else {
                                    // when there is no dragging (just a click), the event is passed
                                    // to the desktop mousearea
                                    desktopMouseArea.clicked(mouse);
                                }

                                windowRect.x = saveState.x;
                                windowRect.y = saveState.y;
                                windowRect.parent = saveState.parent;
                            }
                        }

                        function checkDesktopHover() {
                            if (!windowMouseArea.drag.active)
                                return;

                            var mouse = root.mapFromItem(windowRect, saveState.mouseX, saveState.mouseY);
                            for (var i = 0; i < root.children.length; i++) {
                                var item = root.children[i];
                                if (item.desktopId != undefined && Utils.contains(item, mouse)) {
                                    root.dragId = item.desktopId;
                                    return;
                                }
                            }
                        }

                        onXChanged: checkDesktopHover();
                        onYChanged: checkDesktopHover();
                    }
                }
            }
        }
    }
}
