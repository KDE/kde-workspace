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

    property Item toolBox

    property Item currentLayout: (plasmoid.formFactor == PlasmaCore.Types.Vertical) ? column : row
    onCurrentLayoutChanged: {
        LayoutManager.layout = currentLayout
    }

    property Item dragOverlay

    onDragEnter: {
        var child = currentLayout.childAt(event.x, event.y);
        dndSpacer.intendedPos = child && child.intendedPos ? child.intendedPos : currentLayout.children.length-1;
        LayoutManager.insertBefore(child, dndSpacer);
    }

    onDragMove: {
        var child = currentLayout.childAt(event.x, event.y);
        if (child === dndSpacer) {
            return;
        }
        dndSpacer.parent = root;
        dndSpacer.intendedPos = child && child.intendedPos ? child.intendedPos : currentLayout.children.length-1;
        LayoutManager.insertBefore(child, dndSpacer);
    }
    
    onDragLeave: {
        dndSpacer.parent = root;
    }

    onDrop: {
        dndSpacer.parent = root;
        plasmoid.processMimeData(event.mimeData, event.x, event.y);
    }

    Connections {
        target: plasmoid

        onAppletAdded: {
            var appletX = applet.x;
            var appletY = applet.y;
            var container = appletContainerComponent.createObject(root)
            print("Applet added in test panel: " + applet + " at: " + appletX + ", " + appletY);

            applet.parent = container;
            container.applet = applet;
            applet.anchors.fill = container;
            applet.visible = true;
            container.visible = true;
            container.intendedPos = LayoutManager.order[applet.id] ? LayoutManager.order[applet.id] : -1;
            if (container.intendedPos < 0) {
                var child = currentLayout.childAt(appletX, appletY)
                container.intendedPos = child && child.intendedPos ? child.intendedPos : currentLayout.children.length-1;
            }
            print("We want to insert the new applet in position " + container.intendedPos)

            var position = 0;
            for (var i = 0; i < currentLayout.children.length; ++i) {
                if (currentLayout.children[i].intendedPos !== undefined &&
                    currentLayout.children[i].intendedPos <= container.intendedPos) {
                    position = i+1;
                }
            }

            LayoutManager.insertAt(container, position);
        }

        onAppletRemoved: {
            LayoutManager.save();
        }

        onFormFactorChanged: {
            lastSpacer.parent = root

            if (plasmoid.formFactor == PlasmaCore.Types.Vertical) {
                for (var container in row.children) {
                    var item = row.children[0];
                    item.parent = column
                }
                lastSpacer.parent = column

            } else {
                lastSpacer.parent = row
                for (var container in column.children) {
                    var item = column.children[0];
                    item.parent = row
                }
                lastSpacer.parent = row
            }
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

    Component {
        id: appletContainerComponent
        Item {
            id: container
            visible: false
            property int intendedPos: 0

            Layout.fillWidth: applet && applet.fillWidth
            Layout.fillHeight: applet && applet.fillHeight

            Layout.minimumWidth: (plasmoid.formFactor != PlasmaCore.Types.Vertical ? (applet && applet.minimumWidth > 0 ? applet.minimumWidth : root.height) : root.width)
            Layout.minimumHeight: (plasmoid.formFactor == PlasmaCore.Types.Vertical ? (applet && applet.minimumHeight > 0 ? applet.minimumHeight : root.width) : root.height)

            Layout.preferredWidth: (plasmoid.formFactor != PlasmaCore.Types.Vertical ? (applet && applet.implicitWidth > 0 ? applet.implicitWidth : root.height) : root.width)
            Layout.preferredHeight: (plasmoid.formFactor == PlasmaCore.Types.Vertical ? (applet && applet.implicitHeight > 0 ? applet.implicitHeight : root.width) : root.height)

            Layout.maximumWidth: (plasmoid.formFactor != PlasmaCore.Types.Vertical ? (applet && applet.maximumWidth > 0 ? applet.maximumWidth : root.height) : root.width)
            Layout.maximumHeight: (plasmoid.formFactor == PlasmaCore.Types.Vertical ? (applet && applet.maximumHeight > 0 ? applet.maximumHeight : root.width) : root.height)

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
                if (parent !== currentLayout) {
                    return;
                }
                translation.x = oldX - x
                translation.y = oldX - y
                translAnim.running = true
                oldX = x
                oldY = y
            }
            transform: Translate {
                id: translation
            }
            NumberAnimation {
                id: translAnim
                duration: 250
                easing.type: Easing.InOutQuad
                target: translation
                properties: "x,y"
                to: 0
            }
            
        }
    }

    Item {
        id: lastSpacer
        parent: currentLayout

        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    Item {
        id: dndSpacer
        property int intendedPos
        width: 50
        height: 50
    }

    RowLayout {
        id: row
        anchors {
            fill: parent
            rightMargin: toolBox ? toolBox.width : 0
        }
    }
    ColumnLayout {
        id: column
        anchors {
            fill: parent
            bottomMargin: toolBox ? toolBox.height : 0
        }
    }



    Component.onCompleted: {
        LayoutManager.plasmoid = plasmoid;
        LayoutManager.root = root;
        LayoutManager.layout = currentLayout;
        LayoutManager.restore();
    }
}
