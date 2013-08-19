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


Item {
    id: root
    width: 640
    height: 48

    property Item toolBox

    property Item currentLayout: (plasmoid.formFactor == PlasmaCore.Types.Vertical) ? column : row

    Connections {
        target: plasmoid
        onAppletAdded: {
            lastSpacer.parent = root
            var container = appletContainerComponent.createObject((plasmoid.formFactor == PlasmaCore.Types.Vertical) ? column : row)
            print("Applet added in test panel: " + applet)
            applet.parent = container
            container.applet = applet
            applet.anchors.fill = applet.parent
            applet.visible = true
            container.visible = true
            lastSpacer.parent = currentLayout
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
    }

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

            Layout.maximumWidth: (plasmoid.formFactor != PlasmaCore.Types.Vertical ? (applet && applet.maximumWidth > 0 ? applet.maximumWidth : root.height) : root.width)
            Layout.maximumHeight: (plasmoid.formFactor == PlasmaCore.Types.Vertical ? (applet && applet.maximumHeight > 0 ? applet.maximumHeight : root.width) : root.height)

            property Item applet

            PlasmaComponents.BusyIndicator {
                z: 1000
                visible: applet && applet.busy
                running: visible
                anchors.centerIn: parent
            }
        }
    }

    Item {
        id: lastSpacer

        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    RowLayout {
        id: row
        anchors {
            fill: parent
            rightMargin: toolBox.width
        }
    }
    ColumnLayout {
        id: column
        anchors {
            fill: parent
            bottomMargin: toolBox.height
        }
    }

    Component.onCompleted: {
        print("Test Panel loaded")
        print(plasmoid)
    }
}
