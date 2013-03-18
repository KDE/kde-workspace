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

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

Item {
    id: appletHandle

    z: dragMouseArea.z + 1
    opacity: appletItem.controlsOpacity
    width: appletItem.handleWidth
//     property alias appletAppearance: Item {}

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
            visible: (action && typeof(action) != "undefined") ? action.enabled : false
            action: (applet) ? applet.action("configure") : null
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
            visible: (action && typeof(action) != "undefined") ? action.enabled : false
            action: (applet) ? applet.action("rotate") : null
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
                property int startRotation
                property int prevMove
                property bool skipOne: false;

                onPressed: {
//                     mouse.accepted = true
//                     animationsEnabled = false;
                    startX = mouse.x;
                    startY = mouse.y;
                    startRotation = appletItem.rotation;
                    prevMove = 0;
                    resizeHandle.rotation = -appletItem.rotation;


//                     LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)
                }
                onPositionChanged: {
                    if (skipOne) {
                        skipOne = false;
                        return;
                    }
                    var rot = startRotation%360;
                    var snap = 4;
                    var moved = Math.round(((startY - mouse.y)/1.5) % 360);// + startY - mouse.y;
                    var newRotation = (rot - moved)%360;

                    if (newRotation < 0) {
                        newRotation = newRotation + 360;
                    }
                    if ((moved - prevMove) > 100) {
                        print("skipped: " + prevMove + " " + moved + " " + (moved - prevMove));
                        skipOne = true;
                        //prevMove = moved;
                        return;
                    }
//                     if (Math.abs(appletItem.rotation - newRotation) > 20) {
//                         print("skipped " + Math.abs(appletItem.rotation) + " newRotation " + newRotation );
//                         return;
//                     }
                    snapIt(0);
                    snapIt(90);
                    snapIt(180);
                    snapIt(240);

                    function snapIt(snapTo) {
                        if (newRotation > (snapTo - snap) && newRotation < (snapTo + snap)) {
                            newRotation = snapTo;
                        }
                    }
                    print(" Moved :" + moved +" start: " + startRotation  + " new: " + newRotation);
                    appletItem.rotation = newRotation;
                    prevMove = moved;
                    skipOne = true;
                }
                onReleased: {
                    // save rotation
//                    print("saving...");
                    LayoutManager.saveItem(appletItem);
                }
                Rectangle { color: "red"; opacity: 0.6; visible: debug; anchors.fill: parent; }
            }
        }
        ActionButton {
            svg: configIconsSvg
            z: dragMouseArea.z + 1
            elementId: "configure"
            iconSize: root.iconSize
            visible: (action && typeof(action) != "undefined") ? action.enabled : false
            action: (applet) ? applet.action("configure") : null
            Component.onCompleted: {
                if (action && typeof(action) != "undefined") {
                    action.enabled = true
                }
            }
        }
        ActionButton {
            svg: configIconsSvg
            z: dragMouseArea.z + 1
            elementId: "maximize"
            iconSize: root.iconSize
            visible: (action && typeof(action) != "undefined") ? action.enabled : false
            action: (applet) ? applet.action("run associated application") : null
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
        visible: (action && typeof(action) != "undefined") ? action.enabled : null
        action: (applet) ? applet.action("remove") : null
        z: dragMouseArea.z + 1
        width: handleWidth
        anchors {
            bottom: parent.bottom
            bottomMargin: appletItem.margins.bottom + 2
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
