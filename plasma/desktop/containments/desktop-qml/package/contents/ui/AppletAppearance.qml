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
import org.kde.qtextracomponents 0.1 as QtExtras
import "plasmapackage:/code/LayoutManager.js" as LayoutManager

// PlasmaCore.FrameSvgItem {
//QtExtras.MouseEventListener {
Item {
    id: itemGroup

    property int handleWidth: 24
    property int hoverDelay: 100
    property string category
    property string title
    property bool canResizeHeight: true

    property bool showAppletHandle: false
    property real controlsOpacity: (plasmoid.immutable || !showAppletHandle) ? 0 : 1
    property bool handleShown: true
    property string backgroundHints: "NoBackground"
    property bool hasBackground: false
    property bool expandedHandle: (backgroundHints == "NoBackground" && itemGroup.handleShown)
    property bool animationsEnabled: false
    property int minimumWidth: LayoutManager.cellSize.width
    property int minimumHeight: LayoutManager.cellSize.height
    property int appletHandleWidth: appletHandle.width


    property Item contents: contentsItem
    property alias margins: plasmoidBackground.margins
    property alias imagePath: plasmoidBackground.imagePath

    width: LayoutManager.cellSize.width*2
    height: LayoutManager.cellSize.height
    anchors.rightMargin: -handleWidth*controlsOpacity

    state: expandedHandle ? "expandedhandle" : "normal"
    z: 0
    Rectangle { color: Qt.rgba(0,0,0,0); border.width: 3; border.color: "white"; opacity: 0.5; visible: debug; anchors.fill: parent; }

    QtExtras.MouseEventListener {
        id: mouseListener
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
        width: parent.width+handleWidth;
        hoverEnabled: true

        onContainsMouseChanged: {
            print("Mouse is " + containsMouse);
            if (containsMouse) {
                hoverTracker.restart();
            } else {
                hoverTracker.stop();
                showAppletHandle = false;
            }
        }

        Timer {
            id: hoverTracker
            repeat: false
            interval: hoverDelay
            onTriggered: showAppletHandle = true;
        }
        Rectangle { color: Qt.rgba(0,0,0,0); border.width: 3; border.color: "red"; opacity: 0.5; visible: debug; anchors.fill: parent; }

        PlasmaCore.FrameSvgItem {
            id: noBackgroundHandle
            opacity: (backgroundHints == "NoBackground" && itemGroup.handleShown) ? controlsOpacity : 0
            width: appletHandleWidth + margins.left + margins.right - 4
            imagePath: {
                print("2 applet.backgroundHints " + backgroundHints);

                return backgroundHints == "NoBackground" ? "widgets/translucentbackground" : "";
            }
            anchors {
                top: parent.top
                right: parent.right
                bottom: parent.bottom
                rightMargin: -appletHandleWidth
            }
            Component.onCompleted: {
                //print("applet.backgroundHints " + applet.backgroundHints);
            }
        }

        PlasmaCore.FrameSvgItem {
            id: plasmoidBackground
            property bool suspendAnimation: false
            visible: backgroundHints != "NoBackground"
            imagePath: "widgets/background"
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
            width: showAppletHandle ? parent.width : parent.width-handleWidth;
            Behavior on width {
                enabled: animationsEnabled
                NumberAnimation {
                    duration: suspendAnimation? 0 : 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
        Connections {
            target: plasmoid
            onImmutableChanged: {
                //appletHandle.opacity = plasmoid.immutable ? 0 : 1;
                dragMouseArea.visible = !plasmoid.immutable;
                itemGroup.controlsOpacity = plasmoid.immutable ? 0 : 1;
                //appletContainer.opacity = plasmoid.immutable ? 1.0 : 0.66;
                //imagePath = plasmoid.immutable ? "" : "widgets/background";
                //appletHandleWidth = plasmoid.immutable ? 0: appletHandle.height;
                //applet.backgroundHints = plasmoid.immutable ? "NormalBackground" : "NoBackground";

            }
        }

        MouseArea {
            id: dragMouseArea
            anchors.fill: parent
            //anchors.topMargin: iconSize*1.5
            //anchors.bottomMargin: iconSize*1.5
            //Rectangle { color: "purple"; opacity: .3; anchors.fill: parent; }
            property int lastX
            property int lastY
            //z: appletContainer.z + 10
            onPressed: {
                //FIXME: this shouldn't be necessary
    //             mainFlickable.interactive = false
                itemGroup.z = 999
                animationsEnabled = false
                mouse.accepted = true
                var x = Math.round(itemGroup.x/LayoutManager.cellSize.width)*LayoutManager.cellSize.width
                var y = Math.round(itemGroup.y/LayoutManager.cellSize.height)*LayoutManager.cellSize.height
                LayoutManager.setSpaceAvailable(x, y, itemGroup.width, itemGroup.height, true)

                var globalMousePos = mapToItem(main, mouse.x, mouse.y)
                lastX = globalMousePos.x
                lastY = globalMousePos.y

                //debugFlow.refresh();
                placeHolder.syncWithItem(itemGroup)
                placeHolderPaint.opacity = 1
            }
            onPositionChanged: {
                placeHolder.syncWithItem(itemGroup)

                var globalPos = mapToItem(main, x, y)

                var globalMousePos = mapToItem(main, mouse.x, mouse.y)
                itemGroup.x += (globalMousePos.x - lastX)
                itemGroup.y += (globalMousePos.y - lastY)

                lastX = globalMousePos.x
                lastY = globalMousePos.y
            }
            onReleased: dragEnded()
            function dragEnded()
            {
                repositionTimer.running = false
                placeHolderPaint.opacity = 0
                animationsEnabled = true
                LayoutManager.positionItem(itemGroup)
                LayoutManager.save()
                //debugFlow.refresh()
            }
            //Rectangle { color: "yellow"; opacity: 0.1; visible: debug; anchors.fill: parent; }
        }
        Item {
            id: contentsItem
            z: parent.z+1
            x: 0 + itemGroup.margins.left
            y: 0 + itemGroup.margins.top
            width: itemGroup.width - (itemGroup.margins.left + itemGroup.margins.right)
            height: itemGroup.height - (itemGroup.margins.top + itemGroup.margins.bottom)
            Rectangle { color: "green"; opacity: 1; visible: debug; anchors.fill: parent; }
//             anchors {
//                 left: itemGroup.left
//                 top: itemGroup.top
//                 right: itemGroup.right
//                 bottom: parent.bottom
//                 //topMargin: parent.margins.top+itemGroup.appletHandleWidth
//                 topMargin: itemGroup.margins.top
//                 leftMargin: itemGroup.margins.left
//                 rightMargin: itemGroup.margins.right
//                 bottomMargin: itemGroup.margins.bottom
//             }
        }



//         PlasmaCore.SvgItem {
//             id: resizeHandleSvg
//             opacity: controlsOpacity
//             svg: PlasmaCore.Svg {
//                 imagePath: plasmoid.file("images", "resize-handle.svgz")
//             }
//             width: 24
//             height: 24
//             anchors {
//                 right: parent.right
//                 bottom: parent.bottom
//                 rightMargin: itemGroup.margins.right
//                 bottomMargin: itemGroup.margins.bottom
//             }
//             Rectangle { color: "white"; opacity: 0.4; visible: debug; anchors.fill: parent; }
//         }

        AppletHandle {
            id: appletHandle
        }

        MouseArea {
            id: resizeHandle
            width:  handleWidth+itemGroup.margins.right
            height: width
            property bool resizeTop: false
            z: itemGroup.z+1
            visible: !plasmoid.immutable
            anchors {
                right: parent.right
                //bottom: parent.bottom
                topMargin: itemGroup.margins.top
                //rightMargin: handleWidth
                //rightMargin: -(itemGroup.margins.right*controlsOpacity)
            }
            //anchors.fill: resizeHandleSvg
            property int startX
            property int startY

            onPressed: {
                //itemGroup.z = 999
                mouse.accepted = true
                //FIXME: this shouldn't be necessary
    //             mainFlickable.interactive = false
                //plasmoidBackground.suspendAnimation = true;
                animationsEnabled = false;
                startX = mouse.x;
                startY = mouse.y;
                LayoutManager.setSpaceAvailable(itemGroup.x, itemGroup.y, itemGroup.width, itemGroup.height, true)
                //debugFlow.refresh();
            }
            onPositionChanged: {
                itemGroup.width = Math.max(itemGroup.minimumWidth, itemGroup.width + mouse.x-startX)
                if (itemGroup.canResizeHeight) {
                    itemGroup.y = itemGroup.y + (mouse.y-startY);
                    itemGroup.height = Math.max(itemGroup.minimumHeight, itemGroup.height + startY-mouse.y)
                }
            }
            onReleased: {
                animationsEnabled = true

                LayoutManager.positionItem(itemGroup)
                LayoutManager.save()
                LayoutManager.setSpaceAvailable(itemGroup.x, itemGroup.y, widthAnimation.to, heightAnimation.to, false)
                //debugFlow.refresh();
                plasmoidBackground.suspendAnimation = false;
            }
            Rectangle { color: "blue"; opacity: 0.4; visible: debug; anchors.fill: parent; }
        }

        Rectangle { color: "orange"; opacity: 0.1; visible: debug; anchors.fill: parent; }
    }
    Behavior on controlsOpacity {
        NumberAnimation {
            duration: 250
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on scale {
        NumberAnimation {
            duration: 250
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on x {
        enabled: animationsEnabled
        NumberAnimation {
            duration: 250
            easing.type: Easing.InOutQuad
            onRunningChanged: {
                if (!running) {
                    itemGroup.z = 0
                }
            }
        }
    }
    Behavior on y {
        enabled: animationsEnabled
        NumberAnimation {
            duration: 250
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on width {
        enabled: animationsEnabled
        NumberAnimation {
            id: widthAnimation
            duration: 250
            easing.type: Easing.InOutQuad
            onRunningChanged: {
                if (!running) {
                    itemGroup.z = 0
                }
            }
        }
    }
    Behavior on height {
        enabled: animationsEnabled
        NumberAnimation {
            id: heightAnimation
            duration: 250
            easing.type: Easing.InOutQuad
//             onRunningChanged: {
//                 if (!running) {
//                     mainFlickable.interactive = contentItem.height>mainFlickable.height
//                     if (!mainFlickable.interactive) {
//                         contentScrollTo0Animation.running = true
//                     }
//                 }
//             }
        }
    }
    Component.onCompleted: {
        //width = Math.min(470, 32+itemsList.count*140)
        layoutTimer.running = true
        layoutTimer.restart()
        visible = false
    }
}