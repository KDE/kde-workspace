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

    //z: dragMouseArea.z + 1
    opacity: appletItem.controlsOpacity
    width: appletItem.handleWidth
    height: appletItem.handleHeight

    property int buttonMargin: 6
    property int minimumHeight:  6 * (root.iconSize + buttonMargin)

    signal removeApplet

    function updateHeight() {
        // Does the handle's height fit into the frame?
        var mini = appletHandle.minimumHeight + margins.top + margins.bottom;
        print(" = == = = == updateHeight mini:" + mini)
        if (height > mini) {
            appletItem.handleMerged = true;
            print("merged handle");
            height = appletItem.handleMerged ? appletItem.height : minimumHeight
            print(" height: " + height);
            buttonColumn.anchors.right = appletHandle.right;
        } else {
            appletItem.handleMerged = false;
            print("separate handle");
            //appletHandle.anchors.right = appletHandle.parent.right;
            buttonColumn.anchors.verticalCenter = appletItem.verticalCenter;
            buttonColumn.anchors.top = noBackgroundHandle.top
            buttonColumn.anchors.bottom = noBackgroundHandle.bottom
            buttonColumn.anchors.right = noBackgroundHandle.right
        }
    }
    PlasmaCore.FrameSvgItem {
        id: noBackgroundHandle

        width: handleWidth + margins.left + margins.right - 4
        height: handleMerged ? appletItem.handleHeight + noBackgroundHandle.margins.top + noBackgroundHandle.margins.bottom : appletItem.handleHeight
        visible: opacity > 0
        z: plasmoidBackground.z - 10

        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.right
            leftMargin: handleMerged ? ((1-controlsOpacity) * appletItem.handleWidth) * -1 - appletItem.handleWidth * 2 + 2 : ((1-controlsOpacity) * appletItem.handleWidth) * -1 - appletItem.handleWidth
        }
        opacity: (backgroundHints == "NoBackground" || !handleMerged) ? controlsOpacity : 0
        smooth: true
        imagePath: (backgroundHints == "NoBackground" || !handleMerged) ? "widgets/background" : ""
        Rectangle { color: Qt.rgba(0,0,0,0); border.width: 3; border.color: "orange"; opacity: 1; visible: debug; anchors.fill: parent; }
    }

    Column {
        id: buttonColumn
        width: handleWidth
        anchors {
            top: parent.top
            topMargin: (!appletItem.hasBackground) ? noBackgroundHandle.margins.top : appletItem.margins.top
            bottomMargin: appletItem.margins.bottom
            right: appletItem.handleMerged ? parent.right : noBackgroundHandle.right
            rightMargin: appletItem.handleMerged ? -buttonMargin : noBackgroundHandle.margins.right - buttonMargin
        }
        spacing: buttonMargin*2
        ActionButton {
            svg: configIconsSvg
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
                    mouse.accepted = true
                    animationsEnabled = false;
                    startX = mouse.x;
                    startY = mouse.y;
                    startRotation = appletItem.rotation;
                    prevMove = 0;
                    resizeHandle.rotation = -appletItem.rotation;
                    LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)
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
                        //print("skipped: " + prevMove + " " + moved + " " + (moved - prevMove));
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
            elementId: "size-diagonal-tr2bl" // FIXME should be maximize
            //elementId: "maximize"
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
        width: handleWidth
        anchors {
            bottom: appletItem.handleMerged ? parent.bottom : noBackgroundHandle.bottom
            bottomMargin: appletItem.margins.bottom + 2
            right: appletItem.handleMerged ? parent.right : noBackgroundHandle.right
            rightMargin: appletItem.handleMerged ? -buttonMargin : noBackgroundHandle.margins.right - buttonMargin
        }

        svg: configIconsSvg
        elementId: "close"
        iconSize: root.iconSize
        visible: {
            var a = plasmoid.action("remove");
            return (a && typeof(a) != "undefined") ? a.enabled : false;
        }
        // we don't set action, since we want to catch the button click,
        // animate, and then trigger the "remove" action
        // Triggering the action is handled in the appletItem, we just
        // emit a signal here to avoid the applet-gets-removed-before-we-
        // can-animate it race condition.
        onClicked: {
            appletHandle.removeApplet();
        }
        Component.onCompleted: {
            var a = plasmoid.action("remove");
            if (a && typeof(a) != "undefined") {
                a.enabled = true
            }
        }
    }

    PlasmaCore.Svg {
        id: buttonSvg
        imagePath: "widgets/actionbutton"
    }

    Component.onCompleted: updateHeight()
}
