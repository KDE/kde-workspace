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
import org.kde.plasma.containments 0.1 as PlasmaContainments

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

Item {
    id: appletItem

    property int handleWidth: 24
    property string category

    property bool showAppletHandle: false
    property real controlsOpacity: (plasmoid.immutable || !showAppletHandle) ? 0 : 1
    property bool handleShown: true
    property string backgroundHints: "NoBackground"
    property bool hasBackground: false
    property bool expandedHandle: (backgroundHints == "NoBackground" && appletItem.handleShown)
    property bool animationsEnabled: false
    //property int handleWidth: appletHandle.width

    property int minimumWidth: Math.max(LayoutManager.cellSize.width,
                           appletContainer.minimumWidth +
                           appletItem.contents.anchors.leftMargin +
                           appletItem.contents.anchors.rightMargin)

    property int minimumHeight: Math.max(LayoutManager.cellSize.height,
                            appletContainer.minimumHeight +
                            appletItem.contents.anchors.topMargin +
                            appletItem.contents.anchors.bottomMargin)

    property alias applet: appletContainer.applet
    property alias appletItem: appletItem

    property Item contents: contentsItem
    property alias margins: plasmoidBackground.margins
    property alias imagePath: plasmoidBackground.imagePath

    anchors.rightMargin: -handleWidth*controlsOpacity

    //FIXME: this delay is because backgroundHints gets updated only after a while in qml applets
    Timer {
        id: appletTimer
        interval: 250
        repeat: false
        running: false
        onTriggered: updateBackgroundHints()
    }

    function updateBackgroundHints() {
        // We save the applet's background hints in our own property,
        // then we tell the applet to not render a background, 'cause
        // we'll do it for the applet
        hasBackground = (applet.backgroundHints != "NoBackground");
        if (applet.backgroundHints == -1) {
            appletItem.imagePath = "widgets/background";
            backgroundHints = "StandardBackground";
        } else if (applet.backgroundHints == 2) {
            appletItem.imagePath = "widgets/translucentbackground"
            backgroundHints = "TranslucentBackground";
        } else if (applet.backgroundHints == 0) {
            //appletItem.imagePath = "widgets/translucentbackground"
            backgroundHints = "NoBackground";
        } else {
            backgroundHints = "DefaultBackground";
            appletItem.imagePath = "widgets/background";
        }
        applet.backgroundHints = "NoBackground";
    }

    Rectangle { color: Qt.rgba(0,0,0,0); border.width: 3; border.color: "white"; opacity: 0.5; visible: debug; anchors.fill: parent; }

    QtExtras.MouseEventListener {
        id: mouseListener

        anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
        width: parent.width+handleWidth;
        z: 10

        hoverEnabled: true

        onContainsMouseChanged: {
            animationsEnabled = true;
            //print("Mouse is " + containsMouse);
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
            interval: handleDelay
            onTriggered: showAppletHandle = true;
        }
        Rectangle { color: Qt.rgba(0,0,0,0); border.width: 3; border.color: "red"; opacity: 0.5; visible: debug; anchors.fill: parent; }

        PlasmaCore.FrameSvgItem {
            id: noBackgroundHandle

            opacity: (backgroundHints == "NoBackground" && appletItem.handleShown) ? controlsOpacity : 0
            width: handleWidth + margins.left + margins.right - 4
            imagePath: backgroundHints == "NoBackground" ? "widgets/translucentbackground" : ""
            anchors {
                top: parent.top
                right: parent.right
                bottom: parent.bottom
                rightMargin: -handleWidth
            }
        }

        PlasmaCore.FrameSvgItem {
            id: plasmoidBackground
            visible: backgroundHints != "NoBackground"
            imagePath: "widgets/background"
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
            width: showAppletHandle ? parent.width : parent.width-handleWidth;
            z: mouseListener.z-4
            Behavior on width {
                enabled: animationsEnabled
                NumberAnimation {
                    duration: !animationsEnabled ? 0 : 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
        Connections {
            target: plasmoid
            onImmutableChanged: dragMouseArea.visible = !plasmoid.immutable;
        }

        MouseArea {
            id: dragMouseArea
            anchors.fill: parent
            property int lastX
            property int lastY
            z: contentsItem.z - 2
            onPressed: {
                animationsEnabled = false
                mouse.accepted = true
                var x = Math.round(appletItem.x/LayoutManager.cellSize.width)*LayoutManager.cellSize.width
                var y = Math.round(appletItem.y/LayoutManager.cellSize.height)*LayoutManager.cellSize.height
                LayoutManager.setSpaceAvailable(x, y, appletItem.width, appletItem.height, true)

                var globalMousePos = mapToItem(root, mouse.x, mouse.y)
                lastX = globalMousePos.x
                lastY = globalMousePos.y

                placeHolder.syncWithItem(appletItem)
                placeHolderPaint.opacity = 1
            }
            onPositionChanged: {
                placeHolder.syncWithItem(appletItem)

                var globalPos = mapToItem(root, x, y)

                var globalMousePos = mapToItem(root, mouse.x, mouse.y)
                appletItem.x += (globalMousePos.x - lastX)
                appletItem.y += (globalMousePos.y - lastY)

                lastX = globalMousePos.x
                lastY = globalMousePos.y
            }
            onReleased: {
                repositionTimer.running = false
                placeHolderPaint.opacity = 0
                animationsEnabled = true
                LayoutManager.positionItem(appletItem)
                LayoutManager.save()
            }
        }
        Item {
            id: contentsItem
            z: mouseListener.z+1
            x: 0 + appletItem.margins.left
            y: 0 + appletItem.margins.top
            width: appletItem.width - (appletItem.margins.left + appletItem.margins.right)
            height: appletItem.height - (appletItem.margins.top + appletItem.margins.bottom)

            PlasmaContainments.AppletContainer {
                id: appletContainer
                anchors.fill: parent

                onAppletChanged: {
                    applet.appletDestroyed.connect(appletDestroyed)
                    appletTimer.running = true
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: 250
                        easing.type: Easing.InOutQuad
                    }
                }
                function appletDestroyed() {
                    LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)
                    appletItem.destroy()
                }
                Rectangle { color: "green"; opacity: 1; visible: debug; anchors.fill: parent; }
            }
        }

//     PlasmaCore.SvgItem {
//         id: resizeHandleSvg
//         opacity: controlsOpacity
//         svg: PlasmaCore.Svg {
//             imagePath: plasmoid.file("images", "resize-handle.svgz")
//         }
//         width: 24
//         height: 24
//         anchors {
//             right: parent.right
//             bottom: parent.bottom
//             rightMargin: appletItem.margins.right
//             bottomMargin: appletItem.margins.bottom
//         }
//         Rectangle { color: "white"; opacity: 0.4; visible: debug; anchors.fill: parent; }
//     }

        AppletHandle {
            id: appletHandle
        }

        MouseArea {
            id: resizeHandle

            visible: !plasmoid.immutable
            width:  handleWidth+appletItem.margins.right
            height: width
            z: dragMouseArea.z+1
            anchors {
                right: parent.right
                topMargin: appletItem.margins.top
            }

            property int startX
            property int startY

            onPressed: {
                mouse.accepted = true
                animationsEnabled = false;
                startX = mouse.x;
                startY = mouse.y;
                LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)
            }
            onPositionChanged: {
                appletItem.width = Math.max(appletItem.minimumWidth, appletItem.width + mouse.x-startX)
                appletItem.y = appletItem.y + (mouse.y-startY);
                appletItem.height = Math.max(appletItem.minimumHeight, appletItem.height + startY-mouse.y)
            }
            onReleased: {
                animationsEnabled = true

                LayoutManager.positionItem(appletItem)
                LayoutManager.save()
                LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, widthAnimation.to, heightAnimation.to, false)
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
    Behavior on x {
        enabled: animationsEnabled
        NumberAnimation {
            duration: 250
            easing.type: Easing.InOutQuad
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
        }
    }
    Behavior on height {
        enabled: animationsEnabled
        NumberAnimation {
            id: heightAnimation
            duration: 250
            easing.type: Easing.InOutQuad
        }
    }

    Component.onCompleted: {
        layoutTimer.running = true
        layoutTimer.restart()
        visible = false
    }
}