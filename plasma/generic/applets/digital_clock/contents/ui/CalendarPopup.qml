/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
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
import org.kde.plasma.calendar 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components
import org.kde.plasma.extras 2.0 as PlasmaExtras
Item {
    id: root
    width: parent.width
    height: parent.height
    property string date ;
    property date showDate: new Date()
  
    property alias calendarGrid: calendarGrid
    property real cellWidth: root.width/7
    property real cellHeight: root.height/7.5
 
    property Item selectedItem
    property int week;
    property int firstDay: new Date(showDate.getFullYear(), showDate.getMonth(), 1).getDay()

    function isToday(date) {
        if (date == Qt.formatDateTime(new Date(), "d/M/yyyy")) {
            return true;
        }
        else return false;
    }

    function isTodayMonth() {
        return Qt.formatDateTime(new Date(), "yyyy-MM-dd")
    }

    function eventDate(yearNumber,monthNumber,dayNumber) {
        var d = new Date(yearNumber, monthNumber-1, dayNumber);
        return Qt.formatDate(d, "dddd dd MMM yyyy");
    }

    Calendar {
        id: monthCalendar
        days: 7
        weeks: 6
        startDay: 1
        startDate: "2013-08-01"
        onStartDateChanged: {
            month.text = monthName
            monthYear.text = year
        }
    }

    Column {
        height: parent.height
        width: parent.width 

        Row {
            id: calendarOperations
            anchors {
                left: parent.left
                top: parent.top
                right: parent.right
            }
            spacing: 4

            Components.ToolButton {
                id: monthright
                flat: true;
                text: "<";
                width: height;
                anchors.left: parent.left
                anchors.rightMargin: 20
                onClicked: {
                    monthCalendar.previousMonth()
                }
            }

            Components.ToolButton {
                id: month
                anchors.left: monthright.right
                anchors.right: monthYear.left
                anchors.leftMargin: 20
                Loader {
                    id: menuLoader
                }
                onClicked: {
                    if (menuLoader.source == "") {
                        menuLoader.source = "MonthMenu.qml"
                    } else {
                        //menuLoader.source = ""
                    }
                    menuLoader.item.open(0, height);
                }
                text: monthCalendar.monthName
            }

            Components.ToolButton {
                id: monthYear
                text: monthCalendar.year
                anchors.leftMargin: 20
                anchors.left: month.right
                Components.ToolButton {
                    id: increase
                    text: "^"
                    width: 12
                    height: 12
                    anchors.left: monthYear.right
                    onClicked: monthCalendar.nextYear()
                }
                Components.ToolButton {
                    id: decrease
                    text: "v"
                    width: 12
                    height: 12
                    anchors.left: monthYear.right
                    anchors.top: increase.bottom
                    onClicked: monthCalendar.previousYear()
                }
            }

            Components.ToolButton {
                id: previous
                flat: true;
                text: ">";
                width: height;
                anchors.right: parent.right
                onClicked: {
                    monthCalendar.nextMonth()
                }
            }
        }

        DaysCalendar {
            id: calendarGrid
            anchors {
                left: parent.left
                top: calendarOperations.bottom
                right: parent.right
                bottom: calendarToolbar.top
            }
        }

        Item {
            id: calendarToolbar
            anchors {
                left: parent.left
                right: parent.right
                bottomMargin: 20
                bottom: parent.bottom
            }

            Components.ToolButton {
                id: currentDate
                iconSource: "view-pim-calendar"
                width: height
                onClicked: {
                    monthCalendar.startDate = isTodayMonth();
                }
                PlasmaCore.ToolTip {
                    id: tool
                    target: currentDate
                    mainText: "Select Today"
                }
                anchors {
                    left: parent.left
                }
            }

            Components.TextField {
                id: dateField
                text: date == "" ? Qt.formatDateTime ( new Date(), "d/M/yyyy" ): date
                width: calendarOperations.width/3
                anchors {
                    leftMargin: 20
                    rightMargin: 30
                    left: currentDate.right
                    right: weekField.left
                }
            }

            Components.TextField {
                id: weekField
                text: week == 0 ? monthCalendar.currentWeek(): week
                width: calendarOperations.width/10
                anchors {
                    right: parent.right
                }
            }
        }
    }

}
