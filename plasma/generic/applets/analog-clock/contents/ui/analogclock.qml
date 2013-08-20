/*
 *   Author: Marco Martin <mart@kde.org>
 *   Date: Mon Dec 6 2010, 19:01:32
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
import "logic.js" as Logic

Item {
    id: analogclock
    property int minimumWidth: 250
    property int minimumHeight: 250

    property int hours
    property int minutes
    property int seconds
    property bool showSecondsHand: false
    property bool showTimezone: false

    Component.onCompleted: {
        plasmoid.backgroundHints = "NoBackground";
        plasmoid.addEventListener("dataUpdated", dataUpdated);
        plasmoid.addEventListener("ConfigChanged", configChanged);
        dataEngine("time").connectSource("Local", analogclock, 1000);
    }

    function dataUpdated(source, data) {
        var date = new Date("January 1, 1971 "+data.Time);
        hours = date.getHours();
        minutes = date.getMinutes();
        seconds = date.getSeconds();
        timezoneText.text = data.Timezone;
    }

    function configChanged() {
        showSecondsHand = plasmoid.readConfig("showSecondHand");
        showTimezone = plasmoid.readConfig("showTimezoneString");
    }

    PlasmaCore.Svg {
        id: clockSvg
        imagePath: "widgets/clock"
    }

    Item {
        id: clock
        width: parent.width
        anchors {
            top: parent.top
            bottom: showTimezone ? timezoneBg.top : parent.bottom
        }

        PlasmaCore.SvgItem {
            id: face
            anchors.centerIn: parent
            width: Math.min(parent.width, parent.height)
            height: Math.min(parent.width, parent.height)
            svg: clockSvg
            elementId: "ClockFace"
        }

        PlasmaCore.SvgItem {
            id: center
            width: naturalSize.width
            height: naturalSize.height
            anchors.centerIn: face
            svg: clockSvg
            elementId: "HandCenterScrew"
        }

        PlasmaCore.SvgItem {
            anchors.fill: face
            svg: clockSvg
            elementId: "Glass"
        }
    }

    Hand {
        anchors.topMargin: 3
        elementId: "HourHandShdow"
        rotation: 180 + hours * 30 + (minutes/2)
    }
    Hand {
        elementId: "HourHand"
        rotation: 180 + hours * 30 + (minutes/2)
    }

    Hand {
        anchors.topMargin: 3
        elementId: "MinuteHandShadow"
        rotation: 180 + minutes * 6
    }
    Hand {
        elementId: "MinuteHand"
        rotation: 180 + minutes * 6
    }

    Hand {
        anchors.topMargin: 3
        elementId: "SecondHandShadow"
        rotation: 180 + seconds * 6
        visible: showSecondsHand
    }
    Hand {
        elementId: "SecondHand"
        rotation: 180 + seconds * 6
        visible: showSecondsHand
    }

    PlasmaCore.FrameSvgItem {
        id: timezoneBg
        imagePath: "widgets/background"
        width: timezoneText.width+30
        height: timezoneText.height+30
        anchors.centerIn: timezoneText
        visible: showTimezone
    }
    Text {
        id: timezoneText
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: 10
        }
        visible: showTimezone
    }

    PlasmaCore.Dialog {
        id: calendar
        windowFlags: Qt.Popup
//         mainItem: Calendar {
//             firstDayOfMonth: 4
//             today: "2011-12-07"
//             year: Logic.getYear(today)
//             month: Logic.getMonth(today)
//             day: Logic.getDate(today)
//         }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (!calendar.visible) {
                var pos = calendar.popupPosition(analogclock, Qt.AlignCenter);
                calendar.x = pos.x;
                calendar.y = pos.y;
            }
            calendar.visible = !calendar.visible;
        }
    }
}
