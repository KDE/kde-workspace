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
import "plasmapackage:/code/LayoutManager.js" as LayoutManager

// PlasmaCore.FrameSvgItem {
Item {
    id: itemGroup

    width: LayoutManager.cellSize.width*2
    height: LayoutManager.cellSize.height
    z: 0

    property string category
    property string title
    property bool canResizeHeight: false
    property real controlsOpacity: plasmoid.immutable ? 0 : 1
    property bool handleShown: !plasmoid.immutable
    property string backgroundHints: "NoBackground"
    property bool hasBackground: false
    //property alias immutable: plasmoid.immutable
    //imagePath: "widgets/background"
    property bool animationsEnabled: false
    property int minimumWidth: LayoutManager.cellSize.width
    property int minimumHeight: LayoutManager.cellSize.height
    property int appletHandleWidth: appletHandle.width

    property Item contents: contentsItem
    property alias margins: plasmoidBackground.margins
    property alias imagePath: plasmoidBackground.imagePath

    PlasmaCore.FrameSvgItem {
        id: noBackgroundHandle
        //opacity: (plasmoid.backgroundHints == "NoBackground" && handleShown) ? 1 : 0
        x: parent.width - appletHandleWidth
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
    }

    PlasmaCore.FrameSvgItem {
        id: plasmoidBackground
        visible: backgroundHints != "NoBackground"
        anchors.fill: parent
        anchors.rightMargin: (controlsOpacity * -24)
        imagePath: "widgets/background"

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

    PinchArea {
        anchors.fill: parent
        property variant globalLastPoint1
        property variant globalLastPoint2
        property variant globalStartPoint1
        property variant globalStartPoint2
        onPinchStarted: {
            LayoutManager.setSpaceAvailable(itemGroup.x, itemGroup.y, parent.width, parent.height, true)
            globalLastPoint1 = mapToItem(main, pinch.point1.x, pinch.point1.y)
            globalLastPoint2 = mapToItem(main, pinch.point2.x, pinch.point2.y)
            globalStartPoint1 = globalLastPoint1
            globalStartPoint2 = globalLastPoint2
            dragMouseArea.enabled = false
        }
        onPinchUpdated: {
            var globalPoint1 = mapToItem(main, pinch.point1.x, pinch.point1.y)
            var globalPoint2 = mapToItem(main, pinch.point2.x, pinch.point2.y)

            if (globalPoint1.x == globalPoint2.x) {
                return
            }
            itemGroup.x -= (globalStartPoint2.x > globalStartPoint1.x) ? globalLastPoint1.x - globalPoint1.x : globalLastPoint2.x - globalPoint2.x
            itemGroup.y -= (globalStartPoint2.y > globalStartPoint1.y) ? globalLastPoint1.y - globalPoint1.y : globalLastPoint2.y - globalPoint2.y
print(itemGroup.x+" "+itemGroup.y)
            itemGroup.width = Math.max(itemGroup.minimumWidth, itemGroup.width - (globalStartPoint2.x > globalStartPoint1.x ? 1 : -1)*((globalPoint1.x - globalPoint2.x) - (globalLastPoint1.x - globalLastPoint2.x)))
            itemGroup.height = Math.max(itemGroup.minimumHeight, itemGroup.height - (globalStartPoint2.y > globalStartPoint1.y ? 1 : -1)*((globalPoint1.y - globalPoint2.y) - (globalLastPoint1.y - globalLastPoint2.y)))

            globalLastPoint1 = globalPoint1
            globalLastPoint2 = globalPoint2
            placeHolder.syncWithItem(parent)
        }
        onPinchFinished: {
            dragMouseArea.enabled = true
            dragMouseArea.dragEnded()
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
        z: 9999
        opacity: controlsOpacity
        anchors {
            right: parent.right
            bottom: parent.bottom
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
                itemGroup.height = Math.max(itemGroup.minimumHeight, itemGroup.height + mouse.y-startY)
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

    Item {
        id: appletHandle
        z: appletContainer.z + 1
        opacity: itemGroup.controlsOpacity
        //imagePath: "widgets/extender-dragger"
        //prefix: "root"
        width: 24
        anchors {
            //left: parent.left
            right: parent.right
            top: parent.top
            //leftMargin: parent.margins.left
            bottom: parent.bottom
            rightMargin: parent.margins.right - appletHandleWidth
            bottomMargin: parent.margins.bottom
            topMargin: parent.margins.top
        }
        //height: categoryText.height + margins.top + margins.bottom

        //Rectangle { color: "orange"; anchors.fill: parent; opacity: 0.5 }

        ActionButton {
            svg: configIconsSvg
            elementId: "close"
            iconSize: Math.max(16, plasmoidGroup.appletHandleWidth - 8)
            backgroundVisible: false
            //visible: action.enabled
            action: applet.action("remove")
            z: dragMouseArea.z + 1000
            width: appletHandleWidth
            anchors {
                bottom: parent.bottom
                bottomMargin: 6
                right: parent.right
                rightMargin: -6
            }
    //         Rectangle { color: "green"; opacity: 0.4; anchors.fill: parent; }
            Component.onCompleted: {
                if (action && typeof(action) != "undefined") {
                    action.enabled = true
                }
            }
        }

        Column {
            id: buttonColumn
            width: appletHandleWidth
            anchors {
                top: parent.top
                right: parent.right
                rightMargin: -6
            }
            spacing: 12
            ActionButton {
                svg: configIconsSvg
                z: dragMouseArea.z + 1000
                elementId: "size-diagonal-tr2bl"
                iconSize: Math.max(16, plasmoidGroup.appletHandleWidth - 8)
                backgroundVisible: false
                //visible: action.enabled
                //action: applet.action("configure")
//                 anchors {
//                     left: parent.left
//                     right: parent.right
//                     top: parent.top
//                     bottomMargin: 4
//                 }
                Component.onCompleted: {
                    if (action && typeof(action) != "undefined") {
                        action.enabled = true
                    }
                }
            }
        //         Rectangle { color: "orange"; opacity: 0.4; anchors.fill: parent; }
            ActionButton {
                svg: configIconsSvg
                z: dragMouseArea.z + 1000
                elementId: "rotate"
                iconSize: Math.max(16, plasmoidGroup.appletHandleWidth - 8)
                backgroundVisible: false
                //visible: action.enabled
                //action: applet.action("rotate")
                Component.onCompleted: {
                    if (action && typeof(action) != "undefined") {
                        action.enabled = true
                    }
                }
            }
            ActionButton {
                svg: configIconsSvg
                z: dragMouseArea.z + 1000
                elementId: "configure"
                iconSize: Math.max(16, plasmoidGroup.appletHandleWidth - 8)
                backgroundVisible: false
                //visible: action.enabled
                action: applet.action("configure")
//                 anchors {
//                     left: parent.left
//                     right: parent.right
//                     top: parent.top
//                     bottomMargin: 4
//                 }
                Component.onCompleted: {
                    if (action && typeof(action) != "undefined") {
                        action.enabled = true
                    }
                }
            }
        }
        PlasmaCore.Svg {
            id: buttonSvg
            imagePath: "widgets/actionbutton"
        }

        PlasmaCore.SvgItem {
            id: shadowItem
            svg: buttonSvg
            elementId: "move"
            width: iconSize+13//button.backgroundVisible?iconSize+8:iconSize
            height: width
            //visible: button.backgroundVisible
            anchors {
                top: parent.top
//                 left: parent.left
//                 right: parent.right
                bottom: parent.bottom;
                topMargin: parent.anchors.topMargin
                horizontalCenter: parent.horizontalCenter
//                 leftMargin: height + 2
//                 rightMargin: height + 2
            }
        }
    }
}
