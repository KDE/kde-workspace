/*
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
import org.kde.plasma.calendar 2.0 as PlasmaCalendar
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: calendar

    property int avWidth: (parent.width - (3 * theme.largeSpacing)) / 2
    property int avHeight: parent.height - (2 * theme.largeSpacing)

    //anchors.margins: theme.largeSpacing
    property int spacing: theme.largeSpacing
    property alias borderWidth: monthView.borderWidth
    property alias monthView: monthView

    property bool debug: false

    Item {
        id: agenda
        property QtObject day

        width: avWidth
        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
            leftMargin: spacing
            topMargin: spacing
            bottomMargin: spacing
        }

        Rectangle { anchors.fill: parent; color: "orange"; opacity: 0.2; visible: debug; }

        function dateString(format) {
            var d;
            if (agenda.day != undefined) {
                var day = agenda.day;
                d = new Date(day.yearNumber, day.monthNumber-1, day.dayNumber);
            } else {
                d = new Date();
            }
            var o = Qt.formatDate(d, format);
            return o;
        }

        Connections {
            target: monthView
            onDateChanged: {
                print("Day change caught!");
                var d = monthView.date;
                for (var i = 0; i < d.length; i++) {
                    print(" DD " + d[i]);
                }
                print(" date.day   " + d.dayNumber);
                print(" date.month " + d.monthNumber);
                print(" date.year  " + d.yearNumber);
                agenda.day = d;
            }
        }

        PlasmaComponents.Label {
            id: dayLabel
            height: dayHeading.height + dateHeading.height
            width: paintedWidth
            font.pixelSize: height
            text: agenda.dateString("dd")
            opacity: 0.5
        }

        PlasmaExtras.Heading {
            id: dayHeading
            anchors {
                top: parent.top
                left: dayLabel.right
                right: parent.right
                leftMargin: spacing / 2
            }
            level: 1
            elide: Text.ElideRight
            text: agenda.dateString("dddd")
        }
        PlasmaComponents.Label {
            id: dateHeading
            anchors {
                top: dayHeading.bottom
                left: dayLabel.right
                right: parent.right
                leftMargin: spacing / 2
            }
            //opacity: 0.6
            elide: Text.ElideRight
            text: Qt.locale().standaloneMonthName(agenda.day == null ? new Date().getMonth() : agenda.day.monthNumber - 1)
                             + agenda.dateString(" yyyy")
//             text: {
//                 var day = agenda.day;
//                 var d;
//                 if (day != undefined) {
//                     d = new Date(day.yearNumber, day.monthNumber-1, day.dayNumber);
//                 } else {
//                     d = ne Date();
//                 }
//                 var o = Qt.formatDate(d, "MMM yyyy");
//                 return o
//             }
        }

        ListView {
            id: eventList
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                top: parent.top
                topMargin: monthView.cellHeight + dayHeading.height
            }
            //model: monthView.rows
            model: [ 8, 10, 12, 14, 16, 18 ]

            delegate: Item {
                height: monthView.cellHeight
                width: parent.width
                Rectangle {
                    height: monthView.borderWidth
                    color: theme.textColor
                    opacity: monthView.borderOpacity
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                        leftMargin: spacing
                        rightMargin: spacing
                    }
                }

                PlasmaComponents.Label {
                    id: hourLabel
                    height: paintedHeight
                    font.pixelSize: monthView.cellHeight / 3
                    opacity: 0.5
                    anchors {
                        right: minuteLabel.left
                        verticalCenter: parent.verticalCenter
                    }
                    //text: eventList.model.get(index)
                    text: modelData
                }
                PlasmaComponents.Label {
                    id: minuteLabel
                    x: theme.largeSpacing*2

                    height: paintedHeight
                    font.pixelSize: hourLabel.paintedHeight / 2
                    opacity: hourLabel.opacity
                    //font: theme.smallestFont
                    anchors {
                        top: hourLabel.top
                    }
                    text: "00"
                }
                Rectangle { anchors.fill: parent; color: "orange"; opacity: 0.2;  visible: debug;  }

            }
            Rectangle { anchors.fill: parent; color: "#4bd2ff"; opacity: 0.2;  visible: debug;  }
        }

    }
    Item {
        id: cal
        width: avWidth
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
            rightMargin: spacing
            topMargin: spacing
            bottomMargin: spacing
        }

        Rectangle { anchors.fill: parent; color: "orange"; opacity: 0.2;  visible: debug;  }

        function isCurrentYear(date) {
            var d = new Date();
            if (d.getFullYear() == date.getFullYear()) {
                return true;
            }
            return false;
        }

        PlasmaExtras.Heading {
            id: monthHeading

            anchors {
                top: parent.top
                left: monthView.left
//                 leftMargin: -borderWidth
                right: monthView.right
            }

            level: 1
            //opacity: 0.6
            //text: monthCalendar.monthName + ", " + monthCalendar.year
            text: cal.isCurrentYear(monthView.startDate) ?  monthView.selectedMonth :  monthView.selectedMonth + ", " + monthView.selectedYear
            elide: Text.ElideRight
            Loader {
                id: menuLoader
                property QtObject monthCalendar: monthView.calendar
            }
            MouseArea {
                id: monthMouse
                width: monthHeading.paintedWidth
                //z: height
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                }
                //anchors.fill: parent
                Rectangle { anchors.fill: parent; color: "green"; opacity: 0.2;  visible: debug;  }
                onClicked: {
                    //print("menu ...");
                    if (menuLoader.source == "") {
                        menuLoader.source = "MonthMenu.qml"
                    }
                    menuLoader.item.open(0, height);
                }
            }
        }

        PlasmaCalendar.MonthView {
            id: monthView
            borderOpacity: 0.25
            anchors {
                top: monthHeading.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
        }

    }

    MouseArea {
        id: pin

        /* Allows the user to keep the calendar open for reference */

        width: theme.largeSpacing
        height: width
        anchors {
            top: parent.top
            right: parent.right
        }

        property bool checked: false

        onClicked: {
            pin.checked = !pin.checked;
            plasmoid.hideOnWindowDeactivate = !pin.checked;
        }

        PlasmaComponents.Label {
            width: paintedWidth
            height: paintedHeight
            anchors.centerIn: parent
            text: "✓"
            opacity: pin.checked ? 1 : 0.3
        }
    }
}