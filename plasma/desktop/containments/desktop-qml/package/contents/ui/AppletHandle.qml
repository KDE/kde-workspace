/*
 *   Copyright 2011-2013 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: appletHandle

    z: dragMouseArea.z + 1
    opacity: appletItem.controlsOpacity
    width: appletAppearance.handleWidth

    onHeightChanged: print("handleheight: " + height);

    property int buttonMargin: 6
    property int minimumHeight:  6 * (root.iconSize + buttonMargin)

    Column {
        id: buttonColumn
        width: handleWidth
        anchors {
            top: parent.top
            topMargin: appletItem.margins.top
            bottomMargin: appletItem.margins.bottom
            right: parent.right
            rightMargin: -buttonMargin
        }
        spacing: buttonMargin*2
        ActionButton {
            svg: configIconsSvg
            z: dragMouseArea.z + 1
            elementId: "size-diagonal-tr2bl"
            iconSize: root.iconSize
            visible: action.enabled
            //action: applet.action("configure")
            Component.onCompleted: {
                if (action && typeof(action) != "undefined") {
                    action.enabled = true
                }
            }
        }
        ActionButton {
            svg: configIconsSvg
            z: dragMouseArea.z + 1
            elementId: "rotate"
            iconSize: root.iconSize
            visible: action.enabled
            action: applet.action("rotate")
            Component.onCompleted: {
                if (action && typeof(action) != "undefined") {
                    action.enabled = true
                }
            }
            MouseArea {
                id: resizeHandle

//                 visible: !plasmoid.immutable
//                 width:  handleWidth+appletItem.margins.right
//                 height: width
//                 z: dragMouseArea.z+1
                anchors {
                    fill: parent
                    margins: -buttonMargin
                }

                property int startX
                property int startY

                onPressed: {
//                     mouse.accepted = true
//                     animationsEnabled = false;
                    startX = mouse.x;
                    startY = mouse.y;

//                     LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)
                }
                onPositionChanged: {
                    var rot = appletItem.rotation;
                    var snap = 4;
                    var moved = ((mouse.x - startX)) % 360;// + startY - mouse.y;
                    if (Math.abs(rot - moved) > 20) {
                        return;
                    }
                    snapIt(0);
                    snapIt(90);
                    snapIt(180);
                    snapIt(240);

                    function snapIt(snapTo) {
                        if (moved > (snapTo - snap) && moved < (snapTo + snap)) {
                            moved = snapTo;
                        }
                    }
                    print(" Moved :" + moved);
                    appletItem.rotation = moved;
                }
                onReleased: {
                    // save rotation
                }
                Rectangle { color: "red"; opacity: 0.6; visible: debug; anchors.fill: parent; }
            }
        }
        ActionButton {
            svg: configIconsSvg
            z: dragMouseArea.z + 1
            elementId: "configure"
            iconSize: root.iconSize
            //visible: action.enabled
            action: applet.action("configure")
            Component.onCompleted: {
                if (action && typeof(action) != "undefined") {
                    action.enabled = true
                }
            }
        }
    }

    ActionButton {
        svg: configIconsSvg
        elementId: "close"
        iconSize: root.iconSize
        visible: action.enabled
        action: applet.action("remove")
        z: dragMouseArea.z + 1
        width: handleWidth
        anchors {
            bottom: parent.bottom
            bottomMargin: appletItem.margins.bottom
            right: parent.right
            rightMargin: -buttonMargin
        }

        Component.onCompleted: {
            if (action && typeof(action) != "undefined") {
                action.enabled = true
            }
        }
    }

    PlasmaCore.Svg {
        id: buttonSvg
        imagePath: "widgets/actionbutton"
    }
}
