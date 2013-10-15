/*
*   Copyright 2011 Sebastian Kügler <sebas@kde.org>
*   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
*   Copyright 2013 Kai Uwe Broulik <kde@privat.broulik.de>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2 or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components
import "plasmapackage:/code/logic.js" as Logic

ListView {
    id: view

    property int minimumWidth: isConstrained() ? theme.iconSizes.dialog : 24 // NOTE: Keep in sync with systray
    property int minimumHeight: isConstrained() ? minimumHeight * view.count : 24

    property bool hasBattery: pmSource.data["Battery"]["Has Battery"]

    /*property QtObject pmSource: batterymonitor.pmSource
    property QtObject batteries: batterymonitor.batteries*/

    property bool singleBattery: isConstrained() || !hasBattery

    model: singleBattery ? 1 : batteries

    anchors.fill: parent
    orientation: ListView.Horizontal
    interactive: false

    function isConstrained() {
        // FIXME
        return false;
        //return (plasmoid.formFactor == PlasmaCore.Types.Vertical || plasmoid.formFactor == PlasmaCore.Types.Horizontal);
    }

    Component.onCompleted: {
        print("JOOOOOO" + batteries);
    }

    delegate: Item {
        id: batteryContainer

        property bool hasBattery: view.singleBattery ? batteries.count : model["Plugged in"]
        property int percent: view.singleBattery ? batteries.cumulativePercent : model["Percent"]
        property bool pluggedIn: view.singleBattery ? pmSource.data["AC Adapter"]["Plugged in"] : pmSource.data["AC Adapter"]["Plugged in"] && model["Is Power Supply"]

        // When there are multiple batteries, don't pulse when low for non-powersupply batteries
        // A not properly disconnected Bluetooth mouse for example may still show up with 0%
        // don't unneccessarily alarm the user.
        // When we should show only a single battery (eg. constrained in tray) such batteries will
        // only be shown when there are no primary batteries (eg. on a desktop) and there a potentially
        // failing keyboard or mouse is an urgent situation
        property bool animate: view.singleBattery ? batteries.cumulativePluggedin : model["Is Power Supply"]

        width: view.width/view.count
        height: view.height

        property real iconSize: Math.min(width, height)

        Column {
            anchors.fill: parent

            BatteryIcon {
                id: batteryIcon
                anchors.centerIn: isConstrained() ? parent : undefined
                anchors.horizontalCenter: isConstrained() ? undefined : parent.horizontalCenter
                hasBattery: batteryContainer.hasBattery
                percent: batteryContainer.percent
                pluggedIn: batteryContainer.pluggedIn
                animate: batteryContainer.animate
                height: isConstrained() ? batteryContainer.iconSize : batteryContainer.iconSize - batteryLabel.height
                width: height
            }

            Components.Label {
                id: batteryLabel
                width: parent.width
                height: paintedHeight
                horizontalAlignment: Text.AlignHCenter
                text: percent + "%"// FIXME i18nc("battery percentage below battery icon", "%1%", percent)
                font.pixelSize: Math.max(batteryContainer.iconSize/8, 10)
                visible: true//!isConstrained()
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: plasmoid.expanded = !plasmoid.expanded

        PlasmaCore.ToolTip {
            id: tooltip
            target: mouseArea
            image: batteries.tooltipImage
            subText: batteries.tooltipText
        }
    }
}
