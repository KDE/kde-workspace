/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components

Item {
    id: main
    //property int minimumWidth: time.font.pixelSize * (plasmoid.configuration.showSeconds ? 4 : 3)
    property int minimumWidth: time.paintedWidth
    property int maximumWidth: minimumWidth
    property int minimumHeight
    property int formFactor: plasmoid.formFactor
    property int timePixelSize: theme.defaultFont.pixelSize
    property int timezonePixelSize: theme.smallestFont.pixelSize
    property bool fillWidth: true
    property bool fillHeight: true

    property bool constrained: formFactor == PlasmaCore.Types.Vertical || formFactor == PlasmaCore.Types.Horizontal

    PlasmaCore.DataSource {
        id: dataSource
        engine: "time"
        connectedSources: ["Local"]
        interval: plasmoid.configuration.showSeconds ? 500 : 300000
    }

    Components.Label  {
        id: time
        font.bold: plasmoid.configuration.boldText
        font.italic: plasmoid.configuration.italicText
        font.pixelSize: Math.min(main.width/6, main.height)
        style: Text.Raised; styleColor: "black"
        width: Math.max(paintedWidth,time.paintedWidth)
        opacity: 0.8
        color: "white"
        text: {
            if (plasmoid.configuration.showSeconds) {
                print("WITH Seconds");
                return Qt.formatTime(dataSource.data["Local"]["Time"], timeFormatCorrection(Qt.locale().timeFormat(Locale.LongFormat)));
            } else {
                print("Without Seconds");
                return Qt.formatTime(dataSource.data["Local"]["Time"], Qt.DefaultLocaleShortDate);
            }
        }
        horizontalAlignment: main.AlignHCenter
        verticalAlignment: main.AlignVCenter
        anchors {
            centerIn: parent
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                plasmoid.expanded = !plasmoid.expanded;
                calTimer.start();
            }
            Timer {
                id: calTimer
                interval: 100
                onTriggered: {
                    if (calendarLoader.source == "") {
                        calendarLoader.source = "CalendarView.qml"
                    }
                }
            }
            PlasmaCore.ToolTip {
                id: tooltip
                target: time
                mainText: "Current Time"
                subText: Qt.formatDate(dataSource.data["Local"]["Date"],"dddd, MMM d yyyy") + "\n" + Qt.formatTime(dataSource.data["Local"]["Time"], Qt.DefaultLocaleShortDate)
                image: "preferences-system-time"
            }
        }
    }

    function updateSize() {
        //var maxSize = theme.smallestFont.pixelSize;
        var maxSize = 0;
        var threshold = theme.mSize(theme.defaultFont).height / 2;
        var f = theme.defaultFont;
//         print(" STarting with:  " + theme.mSize(f).height);
        var stop = false;
        //var _ps = maxSize;
        while (!stop) {
//             print(" maxSize.height: " + maxSize);
//             print("Main height: " + main.height);
            if (maxSize + threshold >= main.height) {
                stop = true;
            }
            maxSize = maxSize + 1;
            //print("_ps = " + _ps);
            //f.pointSize = _ps;

        }
        //maxSize = maxSize - 1;
//         print(" then with:  " + theme.mSize(f).height);
        time.font.pixelSize = maxSize;
        //if (time.paintedWidth)
    }

    // Qt's locale puts timezone in the long format by default, remove it if it's present
    function timeFormatCorrection(timeFormatString) {
        var tPosition = timeFormatString.indexOf('t');
        if (!plasmoid.configuration.showTimezone && tPosition != -1) {
            timeFormatString = timeFormatString.replace(" t", "");
        }

        return timeFormatString;
    }

    onWidthChanged: updateSize()
    onHeightChanged: updateSize()
}
