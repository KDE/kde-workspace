// -*- coding: iso-8859-1 -*-
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
import org.kde.plasma.components 2.0 as PlasmaComponents

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

Item {
    id: root

    property Item toolBox

    property bool debug: false
    property int handleDelay: 800
    property real haloOpacity: 0.5

    property int iconSize: 16
    property int iconWidth: iconSize
    property int iconHeight: iconWidth

    onIconHeightChanged: updateGridSize()

    function updateGridSize()
    {
        print("Updategridsize");
        LayoutManager.cellSize.width = root.iconWidth + toolBoxSvg.elementSize("left").width + toolBoxSvg.elementSize("right").width
        LayoutManager.cellSize.height = root.iconHeight + toolBoxSvg.elementSize("top").height + toolBoxSvg.elementSize("bottom").height;
        layoutTimer.restart()
    }

    function addApplet(applet, pos)
    {
        var component = Qt.createComponent("AppletAppearance.qml");
        var e = component.errorString();
        if (e != "") {
            print("Error loading AppletAppearance.qml: " + component.errorString());
        }

        var container = component.createObject(resultsFlow)

        applet.parent = container
        applet.visible = true;

        container.category = "Applet-"+applet.id; // FIXME: undefined in Applet
        container.width = LayoutManager.cellSize.width*6;
        container.height = LayoutManager.cellSize.height*6;
        container.applet = applet;

        LayoutManager.itemGroups[container.category] = container;
        print("Applet " + container.category + " added.");
    }

    PlasmaCore.Svg {
        id: toolBoxSvg
        imagePath: "widgets/toolbox"
        property int rightBorder: elementSize("right").width
        property int topBorder: elementSize("top").height
        property int bottomBorder: elementSize("bottom").height
        property int leftBorder: elementSize("left").width
    }
    PlasmaCore.Svg {
        id: configIconsSvg
        imagePath: "widgets/configuration-icons"
    }

    Item {
        id: resultsFlow
        anchors.fill: parent

        anchors {
            top: parent.top
            topMargin: 5
            horizontalCenter: parent.horizontalCenter
        }

        //This is just for event compression when a lot of boxes are created one after the other
        Timer {
            id: layoutTimer
            repeat: false
            running: false
            interval: 100
            onTriggered: {
                //return; // FIXME
                LayoutManager.resetPositions()
                for (var i=0; i<resultsFlow.children.length; ++i) {
                    var child = resultsFlow.children[i]
                    if (child.enabled) {
                        if (LayoutManager.itemsConfig[child.category]) {
                            var rect = LayoutManager.itemsConfig[child.category]
                            child.x = rect.x
                            child.y = rect.y
                            child.width = rect.width
                            child.height = rect.height
                            child.rotation = rect.rotation
                        } else {
                            child.x = 0
                            child.y = 0
                            child.width = Math.min(470, 32+child.categoryCount*140)
                        }
                        child.visible = true
                        LayoutManager.positionItem(child)
                    } else {
                        child.visible = false
                    }
                }
                LayoutManager.save()
            }
        }
        Component.onCompleted: LayoutManager.resultsFlow = resultsFlow
    }

    Item {
        anchors.fill: resultsFlow
        z: 0

        Item {
            id: placeHolder

            x: -10000 // move offscreen initially to avoid flickering
            width: 100
            height: 100

            property bool animationsEnabled
            property int minimumWidth
            property int minimumHeight
            property Item syncItem

            function syncWithItem(item) {
                syncItem = item
                minimumWidth = item.minimumWidth
                minimumHeight = item.minimumHeight
                repositionTimer.running = true
                if (placeHolderPaint.opacity < 1) {
                    placeHolder.delayedSyncWithItem()
                }
            }

            function delayedSyncWithItem() {
                placeHolder.x = placeHolder.syncItem.x
                placeHolder.y = placeHolder.syncItem.y
                placeHolder.width = placeHolder.syncItem.width
                placeHolder.height = placeHolder.syncItem.height
                //only positionItem here, we don't want to save
                LayoutManager.positionItem(placeHolder)
                LayoutManager.setSpaceAvailable(placeHolder.x, placeHolder.y, placeHolder.width, placeHolder.height, true)
            }

            Timer {
                id: repositionTimer
                interval: 100
                repeat: false
                running: false
                onTriggered: placeHolder.delayedSyncWithItem()
            }
        }

        PlasmaComponents.Highlight {
            id: placeHolderPaint

            x: placeHolder.x + (root.iconSize/2)
            y: placeHolder.y + (root.iconSize/2)
            width: placeHolder.width + (root.iconSize/2)
            height: placeHolder.height - root.iconSize
            z: 0
            visible: false

            property int moveDuration: 75

            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
            Behavior on x {
                enabled: placeHolderPaint.opacity > 0
                NumberAnimation {
                    duration: placeHolderPaint.moveDuration
                    easing.type: Easing.InOutQuad
                }
            }
            Behavior on y {
                enabled: placeHolderPaint.opacity > 0
                NumberAnimation {
                    duration: placeHolderPaint.moveDuration
                    easing.type: Easing.InOutQuad
                }
            }
            Behavior on width {
                enabled: placeHolderPaint.opacity > 0
                NumberAnimation {
                    duration: placeHolderPaint.moveDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
    /*
    ContainmentConfig {
        anchors {
            top: parent.top
            left: parent.left
        }
    }
    */
    PlasmaCore.IconItem {
        width: 24
        height: 24
        source: "list-add"
        MouseArea {
            anchors.fill: parent
            onClicked: {
                print("Add widgets ...");
                plasmoid.action("add widgets").trigger();
            }
        }
    }

    Component.onCompleted: {
        placeHolderPaint.opacity = 0;
        placeHolderPaint.visible = true;
        print("Containment completed.");
        LayoutManager.plasmoid = plasmoid
        updateGridSize()
        plasmoid.containmentType = "CustomContainment"
        //plasmoid.containmentType = "DesktopContainment"
        plasmoid.appletAdded.connect(addApplet)
        LayoutManager.restore()

        for (var i = 0; i < plasmoid.applets.length; ++i) {
            var applet = plasmoid.applets[i]
            addApplet(applet, 0)
        }
    }
}
