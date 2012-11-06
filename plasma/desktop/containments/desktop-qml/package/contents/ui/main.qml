// -*- coding: iso-8859-1 -*-
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
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtras

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

QtExtras.MouseEventListener {
    id: main
    width: 540
    height: 540

    signal minimumWidthChanged
    signal minimumHeightChanged
    signal maximumWidthChanged
    signal maximumHeightChanged
    signal preferredWidthChanged
    signal preferredHeightChanged

    property Item currentGroup
    property int currentIndex: -1

    property Item addResource
    property int iconSize: 16

    property variant availScreenRect: plasmoid.availableScreenRegion(plasmoid.screen)[0]

    property int iconWidth: 22
    property int iconHeight: iconWidth

    //property alias tb: plasmoid.toolBox
    property QtObject toolBox: plasmoid.toolBox


    onIconHeightChanged: updateGridSize()
    onPressed: {
//         if ((mouse.x > toolBox.x && mouse.x < (toolBox.x + toolBox.width)) &&
//             (mouse.y > toolBox.y && mouse.y < (toolBox.y + toolBox.height))) {
//             return;
//         }
        //var tb = plasmoid.toolBox;
        if (toolBox.showing) {
            toolBox.showing = false;
        }
        print("MEL clicked");
    }
    function updateGridSize()
    {
        LayoutManager.cellSize.width = main.iconWidth + toolBoxSvg.elementSize("left").width + toolBoxSvg.elementSize("right").width
        LayoutManager.cellSize.height = main.iconHeight + theme.defaultFont.mSize.height + toolBoxSvg.elementSize("top").height + toolBoxSvg.elementSize("bottom").height + draggerSvg.elementSize("root-top").height + draggerSvg.elementSize("root-bottom").height
        layoutTimer.restart()
    }

//     GridView {
//         id: gridView
//         opacity: 0.3
//         cellWidth: 24
//         cellHeight: 24
//         model: gridModel
//         interactive: false
//         anchors.fill: parent
//         delegate: Rectangle {
//             border { color: "#000000"; width: 1; }
//             anchors.margins: 12
//             width: gridView.cellWidth
//             height: gridView.cellHeight
//             color: Qt.rgba(0, 0, 0, 0)
//             opacity: 0.3
//
//         }
//
//         ListModel {
//             id: gridModel
//             Component.onCompleted: {
//                 var cells  = (main.width / gridView.cellWidth) * (main.height / gridView.cellHeight);
//                 print(" Got " + cells + " Cells. w: " + (availScreenRect.width / gridView.cellWidth) + " h: " + (availScreenRect.height / gridView.cellHeight));
//                 for (i = 0; i < cells*3; i++) {
//                     gridModel.append({"numbor": i, "name":"Jackfruit"});
//                 }
//
//             }
//         }
//     }

    Component.onCompleted: {
        //do it here since theme is not accessible in LayoutManager
        //TODO: icon size from the configuration
        //TODO: remove hardcoded sizes, use framesvg boders
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

    function addApplet(applet, pos)
    {
        var component = Qt.createComponent("PlasmoidGroup.qml")
        var plasmoidGroup = component.createObject(resultsFlow)
        plasmoidGroup.width = LayoutManager.cellSize.width*2
        plasmoidGroup.height = LayoutManager.cellSize.height*2
        plasmoidGroup.applet = applet
        plasmoidGroup.category = "Applet-"+applet.id
        LayoutManager.itemGroups[plasmoidGroup.category] = plasmoidGroup
    }

    PlasmaCore.Svg {
        id: iconsSvg
        imagePath: "widgets/configuration-icons"
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
        id: draggerSvg
        imagePath: "widgets/extender-dragger"
    }

    PlasmaExtras.ResourceInstance {
        id: resourceInstance
    }

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaCore.Svg {
        id: configIconsSvg
        imagePath: "widgets/configuration-icons"
    }

//     PlasmaComponents.ScrollBar {
//         flickableItem: mainFlickable
//         orientation: Qt.Vertical
//         anchors {
//             right: parent.right
//             top: parent.top
//             bottom: parent.bottom
//         }
//     }
    QtExtras.QIconItem {
        id: lockIcon
        height: 128
        width: 128
        icon: "object-locked"
        opacity: plasmoid.immutable ? 0.2 : 0
        anchors { left: parent.left; bottom: parent.bottom; margins: 32; }
        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }
    }

    Connections {
        target: plasmoid
        onImmutableChanged: {
            print("plasmoid.immutable == " + plasmoid.immutable);
            lockIcon.opacity = plasmoid.immutable ? 0.3 : 0.0;
        }

    }

    Flickable {
        id: mainFlickable
        anchors {
            fill: main
            leftMargin: availScreenRect.x
            rightMargin: parent.width - availScreenRect.x - availScreenRect.width
        }
        interactive: false
        contentWidth: mainFlickable.width
        contentHeight: mainFlickable.height

        MouseArea {
            id: contentItem
            width: mainFlickable.width
            height: childrenRect.y+childrenRect.height

            onClicked: {
                resourceInstance.uri = ""
                main.currentIndex = -1
            }

            //FIXME: debug purposes only, remove asap
            /*Flow {
                id: debugFlow
                anchors.fill: resultsFlow
                visible: true
                Repeater {
                    model: 60
                    Rectangle {
                        width: LayoutManager.cellSize.width
                        height: LayoutManager.cellSize.height
                    }
                }
                function refresh()
                {
                    for (var i=0; i<debugFlow.children.length; ++i) {
                        child = debugFlow.children[i]
                        child.opacity = LayoutManager.availableSpace(child.x,child.y, LayoutManager.cellSize.width, LayoutManager.cellSize.height).width>0?0.8:0.3
                    }
                }
            }*/

            Item {
                id: resultsFlow
                //height: Math.min(300, childrenRect.height)
                width: Math.floor(parent.width/LayoutManager.cellSize.width)*LayoutManager.cellSize.width
                height: childrenRect.y+childrenRect.height
                z: 900

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
                        LayoutManager.resetPositions()
                        for (var i=0; i<resultsFlow.children.length; ++i) {
                            child = resultsFlow.children[i]
                            if (child.enabled) {
                                if (LayoutManager.itemsConfig[child.category]) {
                                    var rect = LayoutManager.itemsConfig[child.category]
                                    child.x = rect.x
                                    child.y = rect.y
                                    child.width = rect.width
                                    child.height = rect.height
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
                            //debugFlow.refresh();
                        }
                        LayoutManager.save()
                    }
                }
                Component.onCompleted: {
                    LayoutManager.resultsFlow = resultsFlow
                }
            }
            Item {
                anchors.fill: resultsFlow
                z: 0
                Item {
                    id: placeHolder
                    property bool animationsEnabled
                    width: 100
                    height: 100
                    property int minimumWidth
                    property int minimumHeight
                    property Item syncItem
                    function syncWithItem(item)
                    {
                        syncItem = item
                        minimumWidth = item.minimumWidth
                        minimumHeight = item.minimumHeight
                        repositionTimer.running = true
                        if (placeHolderPaint.opacity < 1) {
                            placeHolder.delayedSyncWithItem()
                        }
                    }
                    function delayedSyncWithItem()
                    {
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
                        interval: 200
                        repeat: false
                        running: false
                        onTriggered: {
                            placeHolder.delayedSyncWithItem()
                        }
                    }
                }
                Rectangle {
                    id: placeHolderPaint
                    x: placeHolder.x + 6
                    y: placeHolder.y + 6
                    width: placeHolder.width - 12
                    height: placeHolder.height - 12
                    z: 0
                    opacity: 0
                    radius: 8
                    smooth: true
                    color: Qt.rgba(1,1,1,0.25)
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                    }
                    Behavior on x {
                        enabled: placeHolderPaint.opacity == 1
                        NumberAnimation {
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                    }
                    Behavior on y {
                        enabled: placeHolderPaint.opacity == 1
                        NumberAnimation {
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                    }
                    Behavior on width {
                        enabled: placeHolderPaint.opacity == 1
                        NumberAnimation {
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
            }
        }
    }
}
