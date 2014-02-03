/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.1
import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.qtextracomponents 2.0
import org.kde.draganddrop 2.0 as DragDrop

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

DragDrop.DropArea {
    id: root
    width: 640
    height: 48

//BEGIN properties
    property int minimumWidth: currentLayout.Layout.minimumWidth
    property int maximumWidth: currentLayout.Layout.maximumWidth
    implicitWidth: currentLayout.implicitWidth

    property Item toolBox

    property Item dragOverlay
//END properties

//BEGIN functions
function addApplet(applet, x, y) {
    var container = appletContainerComponent.createObject(root)
    print("Applet added in test panel: " + applet + applet.title + " at: " + x + ", " + y);

    if (applet.fillWidth) {
        lastSpacer.parent = root;
    }
    applet.parent = container;
    container.applet = applet;
    applet.anchors.fill = container;
    applet.visible = true;
    container.visible = true;

    //is there a DND placeholder? replace it!
    if (dndSpacer.parent === currentLayout) {
        LayoutManager.insertBefore(dndSpacer, container);
        dndSpacer.parent = root;
        return;

    //if the provided position is valid, try it
    } else if (x >= 0 && y >= 0) {
        var index = LayoutManager.insertAtCoordinates(container, x, y);
        print("Applet " + applet.id + " " + applet.title + " was added in position " + index)

    //give up: enqueue the applet after all the others
    } else {
        //if lastspacer is here, enqueue after it
        if (lastSpacer.parent === currentLayout) {
            LayoutManager.insertBefore(lastSpacer, container);
        //else put it in last position
        } else {
            container.parent = currentLayout;
        }
    }
}

//END functions

//BEGIN connections
    Component.onCompleted: {
        LayoutManager.plasmoid = plasmoid;
        LayoutManager.root = root;
        LayoutManager.layout = currentLayout;
        LayoutManager.restore();
    }

    onDragEnter: {
        LayoutManager.insertAtCoordinates(dndSpacer, event.x, event.y)
    }

    onDragMove: {
        LayoutManager.insertAtCoordinates(dndSpacer, event.x, event.y)
    }

    onDragLeave: {
        dndSpacer.parent = root;
    }

    onDrop: {
        plasmoid.processMimeData(event.mimeData, event.x, event.y);
    }

    Connections {
        target: plasmoid

        onAppletAdded: {
            addApplet(applet, x, y);
            LayoutManager.save();
        }

        onAppletRemoved: {
            var flexibleFound = false;
            for (var i = 0; i < currentLayout.children.length; ++i) {
                if (currentLayout.children[i].applet.fillWidth) {
                    flexibleFound = true;
                    break
                }
            }
            if (!flexibleFound) {
                lastSpacer.parent = currentLayout;
            }

            LayoutManager.save();
        }

        onUserConfiguringChanged: {
            if (plasmoid.immutable) {
                return;
            }

            if (plasmoid.userConfiguring) {
                var component = Qt.createComponent("ConfigOverlay.qml");
                dragOverlay = component.createObject(root);
                component.destroy();
            } else {
                dragOverlay.destroy()
            }
        }
    }
//END connections

//BEGIN components
    Component {
        id: appletContainerComponent
        Item {
            id: container
            visible: false

            Layout.fillWidth: applet && applet.fillWidth
            Layout.fillHeight: applet && applet.fillHeight

            Layout.minimumWidth: (plasmoid.formFactor != PlasmaCore.Types.Vertical ? (applet && applet.minimumWidth > 0 ? applet.minimumWidth : root.height) : root.width)
            Layout.minimumHeight: (plasmoid.formFactor == PlasmaCore.Types.Vertical ? (applet && applet.minimumHeight > 0 ? applet.minimumHeight : root.width) : root.height)

            Layout.preferredWidth: (plasmoid.formFactor != PlasmaCore.Types.Vertical ? (applet && applet.implicitWidth > 0 ? applet.implicitWidth : root.height) : root.width)
            Layout.preferredHeight: (plasmoid.formFactor == PlasmaCore.Types.Vertical ? (applet && applet.implicitHeight > 0 ? applet.implicitHeight : root.width) : root.height)

            Layout.maximumWidth: (plasmoid.formFactor != PlasmaCore.Types.Vertical ? (applet && applet.maximumWidth > 0 ? applet.maximumWidth : (Layout.fillWidth ? -1 : root.height)) : (Layout.fillHeight ? -1 : root.width))
            Layout.maximumHeight: (plasmoid.formFactor == PlasmaCore.Types.Vertical ? (applet && applet.maximumHeight > 0 ? applet.maximumHeight : (Layout.fillHeight ? -1 : root.width)) : (Layout.fillWidth ? -1 : root.height))

            property int oldX: x
            property int oldY: y

            property Item applet
            onAppletChanged: {
                if (!applet) {
                    destroy();
                }
            }

            PlasmaComponents.BusyIndicator {
                z: 1000
                visible: applet && applet.busy
                running: visible
                anchors.centerIn: parent
            }
            onXChanged: {
                translation.x = oldX - x
                translation.y = oldY - y
                translAnim.running = true
                oldX = x
                oldY = y
            }
            onYChanged: {
                translation.x = oldX - x
                translation.y = oldY - y
                translAnim.running = true
                oldX = x
                oldY = y
            }
            transform: Translate {
                id: translation
            }
            NumberAnimation {
                id: translAnim
                duration: units.longDuration
                easing.type: Easing.InOutQuad
                target: translation
                properties: "x,y"
                to: 0
            }
        }
    }
//END components

//BEGIN UI elements
    Item {
        id: lastSpacer
        parent: currentLayout

        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    Item {
        id: dndSpacer
        width: (plasmoid.formFactor == PlasmaCore.Types.Vertical) ? currentLayout.width : theme.mSize(theme.defaultFont).width * 10
        height: (plasmoid.formFactor == PlasmaCore.Types.Vertical) ?  theme.mSize(theme.defaultFont).width * 10 : currentLayout.height
    }

    GridLayout {
        id: currentLayout
        property bool isHorizontal: plasmoid.formFactor == PlasmaCore.Types.Horizontal
        anchors {
            fill: parent
            rightMargin: toolBox && isHorizontal? toolBox.width : 0
            bottomMargin: toolBox && !isHorizontal? toolBox.height : 0
        }
        rows: 1
        columns: 1
        //when horizontal layout top-to-bottom, this way it will obey our limit of one row and actually lay out left to right
        flow: isHorizontal ? GridLayout.TopToBottom : GridLayout.LeftToRight

    }
//END UI elements
}
