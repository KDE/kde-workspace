/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
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
QtExtras.MouseEventListener {
    id: itemGroup

    property int hoverDelay: 800
    property string category
    property string title
    property bool canResizeHeight: false
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
    anchors.rightMargin: itemGroup.state == "expandedhandle" ? -appletHandleWidth : 0

    state: expandedHandle ? "expandedhandle" : "normal"
    z: 0
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
            print("applet.backgroundHints " + applet.backgroundHints);
        }
    }

    PlasmaCore.FrameSvgItem {
        id: plasmoidBackground
        visible: backgroundHints != "NoBackground"
        imagePath: "widgets/background"
        anchors {
            fill: parent
            rightMargin: -24*controlsOpacity
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

    Item {
        id: contentsItem
        anchors {
            left: parent.left
            top: parent.top
            right: parent.right
            bottom: parent.bottom
            //topMargin: parent.margins.top+itemGroup.appletHandleWidth
            topMargin: parent.margins.top
            leftMargin: parent.margins.left
            rightMargin: parent.margins.right
            bottomMargin: parent.margins.bottom
        }
    }

    MouseArea {
        id: dragMouseArea
        anchors.fill: appletHandle
        anchors.topMargin: iconSize*1.5
        anchors.bottomMargin: iconSize*1.5
        //Rectangle { color: "purple"; opacity: .3; anchors.fill: parent; }
        property int lastX
        property int lastY
        z: appletContainer.z + 10
        onPressed: {
            //FIXME: this shouldn't be necessary
//             mainFlickable.interactive = false
            itemGroup.z = 999
            animationsEnabled = false
            mouse.accepted = true
            var x = Math.round(parent.x/LayoutManager.cellSize.width)*LayoutManager.cellSize.width
            var y = Math.round(parent.y/LayoutManager.cellSize.height)*LayoutManager.cellSize.height
            LayoutManager.setSpaceAvailable(x, y, parent.width, parent.height, true)

            var globalMousePos = mapToItem(main, mouse.x, mouse.y)
            lastX = globalMousePos.x
            lastY = globalMousePos.y

            //debugFlow.refresh();
            placeHolder.syncWithItem(parent)
            placeHolderPaint.opacity = 1
        }
        onPositionChanged: {
            placeHolder.syncWithItem(parent)

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

    PlasmaCore.SvgItem {
        opacity: controlsOpacity
        svg: PlasmaCore.Svg {
            imagePath: plasmoid.file("images", "resize-handle.svgz")
        }
        width: 24
        height: 24
        anchors {
            right: parent.right
            bottom: parent.bottom
            rightMargin: itemGroup.margins.right
            bottomMargin: itemGroup.margins.bottom
        }
    }
    MouseArea {
        id: resizeHandle
        width: 48
        height: 48
        property bool resizeTop: false
        z: 9999
        opacity: controlsOpacity
        anchors {
            right: parent.right
            //bottom: parent.bottom
            top: parent.top
            rightMargin: -16
        }

        property int startX
        property int startY

        onPressed: {
            itemGroup.z = 999
            mouse.accepted = true
            //FIXME: this shouldn't be necessary
//             mainFlickable.interactive = false
            animationsEnabled = false
            startX = mouse.x
            startY = mouse.y
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
        }
    }
    Component.onCompleted: {
        //width = Math.min(470, 32+itemsList.count*140)
        layoutTimer.running = true
        layoutTimer.restart()
        visible = false
    }
    AppletHandle {
        id: appletHandle
    }
}
