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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1 as QtExtraComponents
import "utils.js" as Utils

Item {
    id: root

    property int minimumWidth: 176
    property int minimumHeight: 88

    property bool dragging: false
    property int dragId

    anchors.fill: parent

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

                QtExtraComponents.QPixmapItem {
                    id: desktopText
                    property string text: pager.showDesktopName ? desktop.desktopName
                                                                : (pager.showDesktopNumber ? desktop.desktopId+1 : "")
                    anchors.centerIn: parent
                    height: nativeHeight
                    width: nativeWidth
                    pixmap: pager.shadowText(text)
                }

                Repeater {
                    model: windows

                    Rectangle {
                        id: windowRect

                        property int windowId: model.windowId

                        x: model.x
                        y: model.y
                        width: model.width
                        height: model.height
                        color: {
                            if (desktop.active) {
                                if (model.active)
                                    return pager.style.windowActiveOnActiveDesktopColor;
                                else
                                    return pager.style.windowInactiveOnActiveDesktopColor;
                            } else {
                                if (model.active)
                                    return pager.style.windowActiveColor;
                                else
                                    return pager.style.windowInactiveColor;
                            }
                        }

                        border.width: 1
                        border.color: model.active ? pager.style.windowActiveBorderColor
                                                   : pager.style.windowInactiveBorderColor

                        QtExtraComponents.QPixmapItem {
                            id: icon
                            anchors.centerIn: parent
                            pixmap: model.icon
                            height: nativeHeight
                            width: nativeWidth
                            visible: pager.showWindowIcons
                        }

                        MouseArea {
                            id: windowMouseArea
                            anchors.fill: parent
                            drag.target: windowRect
                            drag.axis: Drag.XandYAxis
                            drag.minimumX: -parent.width*2
                            drag.maximumX: root.width + parent.width*2
                            drag.minimumY: -parent.height*2
                            drag.maximumY: root.height + parent.height*2

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
