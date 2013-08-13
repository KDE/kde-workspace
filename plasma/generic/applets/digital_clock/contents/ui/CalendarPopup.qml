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
import QtQuick 1.1
import org.kde.pim.calendar 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
import org.kde.plasma.extras 0.1 as PlasmaExtras
Item {
    id: root
    width: parent.width
    height: parent.height
    property string date ;
    property date showDate: new Date()
    property alias list:list
    property alias calendarGrid:calendarGrid
    property alias text:error.text
    property alias text_event:noEvent.text
    property Item selectedItem
    property int week;
    property int firstDay: new Date(showDate.getFullYear(), showDate.getMonth(), 1).getDay()

    function isToday(date) {
        if(date==Qt.formatDateTime(new Date(), "d/M/yyyy")) 
            return true ;
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
        startDay:1
        sorting:Calendar.Ascending
        startDate: "2013-07-01"
        onStartDateChanged: {
            month.text=monthName
            monthYear.text=year
        }
    }

    Row {
        anchors {
            fill:root
        }

        Column {
            id: calendarColumn
            height: parent.height
            width: parent.width / 2

            Row {
                id: calendarOperations
                height: parent.height/15
                width: parent.width
                clip:true
                focus:true
                spacing:10

                Components.ToolButton {
                    flat: true;
                    text: "<";
                    width: height;
                    id:monthright
                    anchors.left:parent.left
                    clip:true
                    onClicked: {
                        monthCalendar.previousMonth()
                    }
                }

                Components.ToolButton {
                    id: month
                    width:50
                    height:24
                    anchors.right:monthYear.left
                    text:monthCalendar.monthName
                    onClicked: {
                        sectionScroll = sectionScrollComponent.createObject(month)
                        sectionScroll.open()
                    }
                    Component {
                        id: sectionScrollComponent
                        Components.ContextMenu {
                            id:sectionScroll
                            visualParent:month
                            Components.MenuItem {
                                text:"January"
                                onClicked: {
                                    monthCalendar.startDate="2013-01-01"
                                }
                            }
                            Components.MenuItem {
                                text:"February"
                                onClicked: {
                                    monthCalendar.startDate="2013-02-01"
                                }
                            }
                            Components.MenuItem {
                                text:"March"
                                onClicked: {
                                    monthCalendar.startDate="2013-03-01"
                                }
                            }
                            Components.MenuItem {
                                text:"April"
                                onClicked: {
                                    monthCalendar.startDate="2013-04-01"
                                }
                            }
                            Components.MenuItem {
                                text:"May"
                                onClicked: {
                                    monthCalendar.startDate="2013-05-01"
                                }
                            }
                            Components.MenuItem {
                                text:"June"
                                onClicked: {
                                    monthCalendar.startDate="2013-06-01"
                                }
                            }
                            Components.MenuItem {
                                text:"July"
                                onClicked: {
                                    monthCalendar.startDate="2013-07-01"
                                }
                            }
                            Components.MenuItem {
                                text:"August"
                                onClicked: {
                                    monthCalendar.startDate="2013-08-01"
                                }
                            }
                            Components.MenuItem {
                                text:"September"
                                onClicked: {
                                    monthCalendar.startDate="2013-09-01"
                                } 
                            }
                            Components.MenuItem {
                                text:"October"
                                onClicked: {
                                    monthCalendar.startDate="2013-10-01"
                                }
                            }
                            Components.MenuItem {
                                text:"November"
                                onClicked: {
                                    monthCalendar.startDate="2013-11-01"
                                }
                            }
                            Components.MenuItem {
                                text:"December"
                                onClicked: {
                                    monthCalendar.startDate="2013-12-01"
                                }
                            }
                        }
                    }
                }

                Components.ToolButton {
                    id: monthYear
                    width:24
                    height:24
                    text:monthCalendar.year
                    anchors.centerIn:parent
                    Components.ToolButton {
                        id: increase
                        text:"^"
                        width:12
                        height:12
                        anchors.left:monthYear.right
                        onClicked:monthCalendar.nextYear()
                    }
                    Components.ToolButton {
                        id: decrease
                        text:"v"
                        width:12
                        height:12
                        anchors.left:monthYear.right
                        anchors.top:increase.bottom
                        onClicked:monthCalendar.previousYear()
                    }
                }

                Components.ToolButton {
                    flat: true;
                    text: ">";
                    width: height;
                    anchors.right:parent.right
                    onClicked: {
                        monthCalendar.nextMonth()
                    }
                }
            }

            DaysCalendar {
                id: calendarGrid
                property Item selectedItem
            }

            Row {
                id: calendarToolbar
                width:calendarOperations.width
                height:parent.height/20
                spacing:calendarOperations.width/15
                clip:true
                focus:true

                Components.ToolButton {
                    id: currentDate
                    iconSource:"view-pim-calendar"
                    width:24
                    height:24
                    onClicked:monthCalendar.startDate=isTodayMonth();
                    PlasmaCore.ToolTip {
                        id: tool
                        target: currentDate
                        mainText:"Select Today"
                    }
                    anchors {
                        left:parent.left
                    }
                }

                Components.TextField {
                    id: dateField
                    text: date==""?Qt.formatDateTime(new Date(), "d/M/yyyy"):date
                    width:calendarOperations.width/3
                    anchors {
                        centerIn:parent
                    }
                }

                Components.TextField {
                    id: weekField
                    text:week==0?monthCalendar.currentWeek():week
                    width:calendarOperations.width/10
                    anchors {
                        right:parent.right
                    }
                }
            }
        }

        PlasmaCore.SvgItem {
            id: line
            svg: PlasmaCore.Svg {
                id: lineSvg
                imagePath: "widgets/line"
            }
            elementId: "vertical-line"
            height: parent.height
            width: lineSvg.elementSize("vertical-line").width
            anchors {
                left:calendarGrid.right
                right:eventColumn.left
            }
        }

        Column {
            id: eventColumn
            height: parent.height
            width: parent.width / 2

            Components.Label {
                id: error
                text:""
                visible: true
                font.weight:Font.Bold
                font.bold:true
            }

            Components.Label {
                id:noEvent
                text: ""
                visible: true
                font.italic:true
            }

            PlasmaExtras.ScrollArea {
                anchors.fill:parent
                anchors.topMargin:40
                EventList {
                    id:list
                }
            }
        }
    }
}
