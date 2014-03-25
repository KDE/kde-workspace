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
import org.kde.kquickcontrolsaddons 2.0

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

MouseArea {
    id: configurationArea
    z: 1000
    anchors {
        fill: parent
        rightMargin: (plasmoid.formFactor !== PlasmaCore.Types.Vertical) ? toolBox.width : 0
        bottomMargin: (plasmoid.formFactor === PlasmaCore.Types.Vertical) ? toolBox.height : 0
    }
    hoverEnabled: true

    property bool isResizingLeft: false
    property bool isResizingRight: false
    property Item currentApplet

    property int lastX
    property int lastY

    onPositionChanged: {
        if (currentApplet && currentApplet.applet &&
            currentApplet.applet.pluginName == "org.kde.plasma.panelspacer") {
            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                if ((mouse.y - handle.y) < units.largeSpacing ||
                    (mouse.y - handle.y) > (handle.height - units.largeSpacing)) {
                    configurationArea.cursorShape = Qt.SizeVerCursor;
                } else {
                    configurationArea.cursorShape = Qt.ArrowCursor;
                }
            } else {
                if ((mouse.x - handle.x) < units.largeSpacing ||
                    (mouse.x - handle.x) > (handle.width - units.largeSpacing)) {
                    configurationArea.cursorShape = Qt.SizeHorCursor;
                } else {
                    configurationArea.cursorShape = Qt.ArrowCursor;
                }
            }
        } else {
            configurationArea.cursorShape = Qt.ArrowCursor;
        }

        if (pressed) {
            if (currentApplet.applet.pluginName == "org.kde.plasma.panelspacer") {

                if (isResizingLeft) {
                    if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                        handle.y += (mouse.y - lastY);
                        handle.height = currentApplet.height + (currentApplet.y - handle.y);
                    } else {
                        handle.x += (mouse.x - lastX);
                        handle.width = currentApplet.width + (currentApplet.x - handle.x);
                    }

                    lastX = mouse.x;
                    lastY = mouse.y;
                    return;

                } else if (isResizingRight) {
                    if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                        handle.height = mouse.y - handle.y;
                    } else {
                        handle.width = mouse.x - handle.x;
                    }

                    lastX = mouse.x;
                    lastY = mouse.y;
                    return;
                }
            }

            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                currentApplet.y += (mouse.y - lastY);
                handle.y = currentApplet.y;
            } else {
                currentApplet.x += (mouse.x - lastX);
                handle.x = currentApplet.x;
            }

            lastX = mouse.x;
            lastY = mouse.y;

            var item = currentLayout.childAt(mouse.x, mouse.y);

            if (item && item !== placeHolder) {
                placeHolder.width = item.width;
                placeHolder.height = item.height;
                placeHolder.parent = configurationArea;
                var posInItem = mapToItem(item, mouse.x, mouse.y);

                if ((plasmoid.formFactor === PlasmaCore.Types.Vertical && posInItem.y < item.height/2) ||
                    (plasmoid.formFactor !== PlasmaCore.Types.Vertical && posInItem.x < item.width/2)) {
                    LayoutManager.insertBefore(item, placeHolder);
                } else {
                    LayoutManager.insertAfter(item, placeHolder);
                }
            }

        } else {
            var item = currentLayout.childAt(mouse.x, mouse.y);
            if (root.dragOverlay && item && item !== lastSpacer) {
                root.dragOverlay.currentApplet = item;
            } else {
                root.dragOverlay.currentApplet = null;
            }
        }
    }
    onCurrentAppletChanged: {
        if (!root.dragOverlay.currentApplet) {
            return;
        }

        handle.x = currentApplet.x;
        handle.y = currentApplet.y;
        handle.width = currentApplet.width;
        handle.height = currentApplet.height;
    }
    onPressed: {
        if (!root.dragOverlay.currentApplet) {
            return;
        }

        if (currentApplet.applet.pluginName == "org.kde.plasma.panelspacer") {
            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                if ((mouse.y - handle.y) < units.largeSpacing) {
                    configurationArea.isResizingLeft = true;
                    configurationArea.isResizingRight = false;
                } else if ((mouse.y - handle.y) > (handle.height - units.largeSpacing)) {
                    configurationArea.isResizingLeft = false;
                    configurationArea.isResizingRight = true;
                } else {
                    configurationArea.isResizingLeft = false;
                    configurationArea.isResizingRight = false;
                }

            } else {
                if ((mouse.x - handle.x) < units.largeSpacing) {
                    configurationArea.isResizingLeft = true;
                    configurationArea.isResizingRight = false;
                } else if ((mouse.x - handle.x) > (handle.width - units.largeSpacing)) {
                    configurationArea.isResizingLeft = false;
                    configurationArea.isResizingRight = true;
                } else {
                    configurationArea.isResizingLeft = false;
                    configurationArea.isResizingRight = false;
                }
            }
        }

        lastX = mouse.x;
        lastY = mouse.y;
        placeHolder.width = currentApplet.width;
        placeHolder.height = currentApplet.height;
        LayoutManager.insertBefore(currentApplet, placeHolder);
        currentApplet.parent = root;
        currentApplet.z = 900;
    }
    onReleased: {
        if (!root.dragOverlay.currentApplet) {
            return;
        }

        if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
            currentApplet.applet.configuration.length = handle.height;
        } else {
            currentApplet.applet.configuration.length = handle.width;
        }

        configurationArea.isResizingLeft = false;
        configurationArea.isResizingRight = false;

        LayoutManager.insertBefore(placeHolder, currentApplet);
        placeHolder.parent = configurationArea;
        currentApplet.z = 1;

        handle.x = currentApplet.x;
        handle.y = currentApplet.y;
        handle.width = currentApplet.width;
        handle.height = currentApplet.height;
        LayoutManager.save();
    }
    Item {
        id: placeHolder
        visible: configurationArea.containsMouse
        Layout.fillWidth: currentApplet ? currentApplet.Layout.fillWidth : false
        Layout.fillHeight: currentApplet ? currentApplet.Layout.fillHeight : false
    }

    Connections {
        target: currentApplet
        onXChanged: handle.x = currentApplet.x
        onYChanged: handle.y = currentApplet.y
        onWidthChanged: handle.width = currentApplet.width
        onHeightChanged: handle.height = currentApplet.height
    }

    Rectangle {
        id: handle
        visible: configurationArea.containsMouse
        color: theme.backgroundColor
        radius: 3
        opacity: currentApplet ? 0.5 : 0
        PlasmaCore.IconItem {
            source: "transform-move"
            width: Math.min(parent.width, parent.height)
            height: width
            anchors.centerIn: parent
        }
        Rectangle {
            anchors {
                left: parent.left
                top: parent.top
                bottom: (plasmoid.formFactor !== PlasmaCore.Types.Vertical) ? parent.bottom : undefined
                right: (plasmoid.formFactor !== PlasmaCore.Types.Vertical) ? undefined : parent.right
            }
            visible: currentApplet.applet.pluginName == "org.kde.plasma.panelspacer"
            width: units.largeSpacing
            height: units.largeSpacing
            color: theme.textColor
        }
        Rectangle {
            anchors {
                right: parent.right
                top: (plasmoid.formFactor !== PlasmaCore.Types.Vertical) ? parent.top : undefined
                bottom: parent.bottom
                left: (plasmoid.formFactor !== PlasmaCore.Types.Vertical) ? undefined : parent.left
            }
            visible: currentApplet.applet.pluginName == "org.kde.plasma.panelspacer"
            width: units.largeSpacing
            height: units.largeSpacing
            color: theme.textColor
        }
        Behavior on x {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on y {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on width {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on height {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on opacity {
            NumberAnimation {
                duration: units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
}
