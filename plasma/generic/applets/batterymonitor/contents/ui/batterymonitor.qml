/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import "plasmapackage:/code/logic.js" as Logic

Item {
    id: batterymonitor
    property int minimumWidth: dialogItem.width
    property int minimumHeight: dialogItem.height

    property bool show_remaining_time: false

    Component.onCompleted: {
        Logic.updateCumulative();
        Logic.updateTooltip();
        plasmoid.addEventListener('ConfigChanged', configChanged);
    }

    function configChanged() {
        show_remaining_time = plasmoid.readConfig("showRemainingTime");
    }

    property Component compactRepresentation: Component {
        ListView {
            id: view

            property int minimumWidth
            property int minimumHeight

            property bool showOverlay: false
            property bool showMultipleBatteries: false
            property bool hasBattery: pmSource.data["Battery"]["Has Battery"]

            property QtObject pmSource: plasmoid.rootItem.pmSource
            property QtObject batteries: plasmoid.rootItem.batteries

            property bool singleBattery: isConstrained() || !showMultipleBatteries || !hasBattery
            
            model: singleBattery ? 1 : batteries

            PlasmaCore.Theme { id: theme }

            anchors.fill: parent
            orientation: ListView.Horizontal

            function isConstrained() {
                return (plasmoid.formFactor == Vertical || plasmoid.formFactor == Horizontal);
            }

            Component.onCompleted: {
                if (!isConstrained()) {
                    minimumWidth = 32;
                    minimumHeight = 32;
                }
                plasmoid.addEventListener('ConfigChanged', configChanged);
            }

            function configChanged() {
                showOverlay = plasmoid.readConfig("showBatteryString");
                showMultipleBatteries = plasmoid.readConfig("showMultipleBatteries");
            }

            delegate: Item {
                id: batteryContainer

                property bool hasBattery: view.singleBattery ? batteries.cumulativePluggedin : model["Plugged in"]
                property int percent: view.singleBattery ? batteries.cumulativePercent : model["Percent"]
                property bool pluggedIn: pmSource.data["AC Adapter"]["Plugged in"]

                width: view.width/view.count
                height: view.height

                property real size: Math.min(width, height)

                BatteryIcon {
                    id: batteryIcon
                    monochrome: true
                    hasBattery: parent.hasBattery
                    percent: parent.percent
                    pluggedIn: parent.pluggedIn
                    width: size; height: size
                    anchors.centerIn: parent
                }

                Rectangle {
                    id: labelRect
                    // should be 40 when size is 90
                    width: Math.max(parent.size*4/9, 35)
                    height: width/2
                    anchors.centerIn: parent
                    color: theme.backgroundColor
                    border.color: "grey"
                    border.width: 2
                    radius: 4
                    opacity: hasBattery ? (showOverlay ? 0.7 : (isConstrained() ? 0 : mouseArea.containsMouse*0.7)) : 0

                    Behavior on opacity { NumberAnimation { duration: 100 } }
                }

                Text {
                    id: overlayText
                    text: i18nc("overlay on the battery, needs to be really tiny", "%1%", percent);
                    color: theme.textColor
                    font.pixelSize: Math.max(parent.size/8, 11)
                    anchors.centerIn: labelRect
                    // keep the opacity 1 when labelRect.opacity=0.7
                    opacity: labelRect.opacity/0.7
                }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: plasmoid.togglePopup()

                PlasmaCore.ToolTip {
                    id: tooltip
                    target: mouseArea
                    image: "battery"
                    subText: batteries.tooltipText
                }
            }
        }
    }

    property QtObject pmSource: PlasmaCore.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: sources
        onDataChanged: {
            Logic.updateCumulative();
            Logic.updateTooltip();

            var status = "PassiveStatus";
            if (batteries.cumulativePluggedin) {
                if (batteries.cumulativePercent <= 10) {
                    status = "NeedsAttentionStatus";
                } else if (!batteries.allCharged) {
                    status = "ActiveStatus";
                }
            }
            plasmoid.status = status;
        }
    }

    property QtObject batteries: PlasmaCore.DataModel {
        id: batteries
        dataSource: pmSource
        sourceFilter: "Battery[0-9]+"

        property int cumulativePercent
        property bool cumulativePluggedin
        // true  --> all batteries charged
        // false --> one of the batteries charging/discharging
        property bool allCharged
        property string tooltipText
    }

    PopupDialog {
        id: dialogItem
        batteryData: batteries
        pluggedIn: pmSource.data["AC Adapter"]["Plugged in"]
        screenBrightness: pmSource.data["PowerDevil"]["Screen Brightness"]
        remainingMsec: parent.show_remaining_time ? Number(pmSource.data["Battery"]["Remaining msec"]) : 0
        showSuspendButton: pmSource.data["Sleep States"]["Suspend"]
        showHibernateButton: pmSource.data["Sleep States"]["Hibernate"]
        onSuspendClicked: {
            plasmoid.togglePopup();
            service = pmSource.serviceForSource("PowerDevil");
            var operationName = Logic.callForType(type);
            operation = service.operationDescription(operationName);
            service.startOperationCall(operation);
        }
        onBrightnessChanged: {
            service = pmSource.serviceForSource("PowerDevil");
            operation = service.operationDescription("setBrightness");
            operation.brightness = screenBrightness;
            service.startOperationCall(operation);
        }
        property int cookie1: -1
        property int cookie2: -1
        onPowermanagementChanged: {
            service = pmSource.serviceForSource("PowerDevil");
            if (checked) {
                var op1 = service.operationDescription("stopSuppressingSleep");
                op1.cookie = cookie1;
                var op2 = service.operationDescription("stopSuppressingScreenPowerManagement");
                op2.cookie = cookie2;

                var job1 = service.startOperationCall(op1);
                job1.finished.connect(function(job) {
                    cookie1 = -1;
                });

                var job2 = service.startOperationCall(op2);
                job1.finished.connect(function(job) {
                    cookie2 = -1;
                });
            } else {
                var reason = i18n("The battery applet has enabled system-wide inhibition");
                var op1 = service.operationDescription("beginSuppressingSleep");
                op1.reason = reason;
                var op2 = service.operationDescription("beginSuppressingScreenPowerManagement");
                op2.reason = reason;

                var job1 = service.startOperationCall(op1);
                job1.finished.connect(function(job) {
                    cookie1 = job.result;
                });

                var job2 = service.startOperationCall(op2);
                job1.finished.connect(function(job) {
                    cookie2 = job.result;
                });
            }
        }
    }
}
