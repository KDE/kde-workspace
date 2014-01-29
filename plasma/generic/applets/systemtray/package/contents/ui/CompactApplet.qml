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

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root
    objectName: "CompactApplet.qml"

    Layout.minimumWidth: compactRepresentation && compactRepresentation.minimumWidth !== undefined ? compactRepresentation.minimumWidth : -1
    Layout.minimumHeight: compactRepresentation && compactRepresentation.minimumHeight !== undefined ? compactRepresentation.minimumHeight : -1

    Layout.maximumWidth: compactRepresentation && compactRepresentation.maximumWidth !== undefined ? compactRepresentation.maximumWidth : -1
    Layout.maximumHeight: compactRepresentation && compactRepresentation.maximumHeight !== undefined ? compactRepresentation.maximumHeight : -1

    implicitWidth: compactRepresentation && compactRepresentation.implicitWidth !== undefined ? compactRepresentation.implicitWidth : -1
    implicitHeight: compactRepresentation && compactRepresentation.implicitHeight !== undefined ? compactRepresentation.implicitHeight : -1

    Layout.fillWidth: compactRepresentation && compactRepresentation.fillWidth !== undefined ? compactRepresentation.fillWidth : false
    Layout.fillHeight: compactRepresentation && compactRepresentation.fillHeight !== undefined ? compactRepresentation.fillHeight : false

    property Item applet
    property Item compactRepresentation

    onCompactRepresentationChanged: {
        compactRepresentation.parent = root
        compactRepresentation.anchors.fill = root
        root.visible = true
    }
}
