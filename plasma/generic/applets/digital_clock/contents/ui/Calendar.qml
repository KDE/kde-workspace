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
    id:root
    width: parent.width
    height: parent.height
    property string date ;
    property date showDate: new Date()
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
        var d = new Date(yearNumber, monthNumber, dayNumber);
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
            id:col
            height: parent.height
            width: parent.width / 2
            anchors {
                top:parent.top
                bottom:parent.bottom
            }
            Row {
                id:rect1
                height: parent.height/10
                width: parent.width
                Components.ToolButton {
                    flat: true;
                    text: "<";
                    width: 24;
                    height: 24;
                    id:monthright
                    onClicked: {
                        monthCalendar.previousMonth()
                    }
                }
                Components.ToolButton {
                    id:month
                    width:50
                    height:24
                    anchors.leftMargin:rect1.width/3
                    anchors.rightMargin:rect1.width/5
                    anchors.left:monthright.right
                    text:monthCalendar.monthName
                    onClicked: {
                        sectionScroll = sectionScrollComponent.createObject(month)
                        sectionScroll.open()
                    }
                    Component {
                        id:sectionScrollComponent
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
                    id:monthYear
                    width:24
                    height:24
                    text:monthCalendar.year
                    anchors.left:month.right
                    anchors.leftMargin:rect1.width/7
                    anchors.rightMargin:rect1.width/3
                    Components.ToolButton {
                        id:increase
                        text:"^"
                        width:12
                        height:12
                        anchors.left:monthYear.right
                        onClicked:monthCalendar.nextYear()
                    }
                    Components.ToolButton {
                        id:decrease
                        text:"v"
                        width:12
                        height:12
                        anchors.left:monthYear.right
                        anchors.top:increase.bottom
                        onClicked:monthCalendar.previousYear()
                    }
                }
                Components.ToolButton {
                    id:next1
                    flat: true;
                    text: ">";
                    width: 24;
                    height: 24;
                    anchors.left:monthYear.right
                    anchors.right:rect1.right
                    anchors.leftMargin:grid.width/4
                    onClicked: {
                        monthCalendar.nextMonth()
                    }
                }
            }
            Row {
                id:grid
                width:col.width
                height:parent.height-rect1.height-riw.height
                Column {
                    width: grid.width/8
                    height: parent.height
                    Repeater {
                        width: grid.width/8
                        height: parent.height
                        model: monthCalendar.weeksModel
                        Rectangle {
                            id:r
                            width: grid.width/8
                            height:grid.height/monthCalendar.weeks
                            color: "transparent"
                            Components.Label {
                                id:weekNumber
                                anchors.centerIn: parent
                                text: modelData + 1
                                opacity:0.5
                            }
                        }
                    }
                }
                Grid {
                    id:gv
                    columns:monthCalendar.days
                    rows:monthCalendar.weeks//dayLabels.rows
                    width:grid.width*7/8
                    height:parent.height
                    spacing:0
                    property Item selectedItem
                    Repeater {
                        model: monthCalendar.days
                        Rectangle {
                            color: "transparent"
                            width:grid.width/8
                            height: 20
                            Components.Label {
                                text: Qt.formatDate(new Date(showDate.getFullYear(), showDate.getMonth(), index - firstDay +1), "ddd");
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                    Repeater {
                        id:repeater
                        model:monthCalendar.model
                        Rectangle {
                            id:myRectangle
                            width:(grid.width*7/8)/monthCalendar.days
                            height:grid.height/monthCalendar.weeks
                            color:(dateMouse.containsMouse)?"#eeeeee":"transparent"
                            border.color:gv.selectedItem == myRectangle ? "black" : "transparent"
                            Rectangle {
                                width: (grid.width*7/8)/monthCalendar.days
                                height:grid.height/monthCalendar.weeks
                                color:"transparent"
                                opacity:isToday(dayNumber+"/"+monthNumber+"/"+yearNumber)?1:0;
                                border.color:"blue"
                            }
                            Components.Label {
                                id:label
                                anchors.centerIn: parent
                                text:dayNumber
                                font.bold:(containsEventItems)||(containsTodoItems) ? true:false
                                opacity: (isPreviousMonth || isNextMonth || dateMouse.containsMouse) ? 0.5 : 1.0
                            }
                            MouseArea {
                                id:dateMouse
                                anchors.fill:parent
                                hoverEnabled:true
                                onEntered: {
                                    monthCalendar.setSelectedDay(yearNumber, monthNumber, dayNumber);
                                    list.model=monthCalendar.selectedDayModel
                                    if(list.count==0) {
                                        list.model=monthCalendar.upcomingEventsModel
                                    }
                                }
                                onClicked: {
                                    monthCalendar.upcommingEventsFromDay(yearNumber, monthNumber, dayNumber);
                                    var rowNumber = Math.floor(index / 7)   ;
                                    week=1+monthCalendar.weeksModel[rowNumber];
                                    date=dayNumber+"/"+monthNumber+"/"+yearNumber
                                    error.text=(containsEventItems)||(containsTodoItems)?"":eventDate(yearNumber,monthNumber,dayNumber)
                                    errorl.text=(containsEventItems)||(containsTodoItems)?"":" No events found on this day ";
                                    gv.selectedItem=myRectangle
                                }
                            }
                        }
                    }
                }
            }
            
            Rectangle {
                id:test
                width:parent.width
                height:20
                color:"transparent"
                Row {
                    id:riw
                    width:rect1.width
                    height:20
                    spacing:rect1.width/10
                    anchors {
                        left:test.left
                        right:test.right
                        verticalCenter:test.verticalCenter
                    }
                    Components.ToolButton {
                        id:currentDate
                       iconSource:"view-pim-calendar"
                        width:24
                        height:24
                          MouseArea {
                            id:mouse2
                            hoverEnabled:true
                            anchors.fill: parent
                            onClicked: {
                                monthCalendar.startDate=isTodayMonth();
                            }
                            PlasmaCore.ToolTip {
                                id: tool
                                target: mouse2
                                mainText:"Select Today"
                            }
                        }
                    }
                    Components.TextField {
                        id:dateField
                        text: date
                        width:rect1.width/3
                    }
                    Components.TextField {
                        id:weekField
                        text:week
                        width:rect1.width/3
                    }
                }
            }
        }
        
        PlasmaCore.SvgItem {
            id:line
            svg: PlasmaCore.Svg {
                id: lineSvg
                imagePath: "widgets/line"
            }
            elementId: "vertical-line"
            height: parent.height
            width: lineSvg.elementSize("vertical-line").width
            anchors {
                left:grid.right
                right:rig.left
            }
        }

        Column {
            id:rig
            height: parent.height
            width: parent.width / 2
            Components.Label {
                id:error
                text: ""
                visible: true
                font.weight:Font.Bold
                font.bold:true
            }
            Components.Label {
                id:errorl
                text: ""
                visible: true
                font.italic:true
            }
            ListView {
                id:list
                height: parent.height
                width: (parent.width / 4)-(scrollBar.visible ? scrollBar.width : 0)
                model: monthCalendar.upcomingEventsModel
                delegate: Rectangle {
                    width: parent.width-(scrollBar.visible ? scrollBar.width : 0)
                    height: 40
                    color: "transparent"
                    
                    Row {
                        spacing:4
                        width: parent.width-(scrollBar.visible ? scrollBar.width : 0)
                        height:50
                        Rectangle {
                            id:circle1
                            width:6
                            height:6
                            color:"black"
                            border.width:1
                            radius:width*0.5
                            anchors.verticalCenter:incidence.verticalCenter
                        }
                        Rectangle {
                            id:incidence
                            width: parent.width-(scrollBar.visible ? scrollBar.width : 0)
                            height:20
                            color:"transparent"
                            Components.Label {
                                id:sum
                                text:  mimeType.split('.')[mimeType.split('.').length - 1]  + ": \n"+ summary 
                                font.capitalization:Font.Capitalize
                                font.italic:true
                            }
                        }
                    }
                }
                section.property:"startDate"
                section.delegate: Rectangle {
                    id:sect
                    width: parent.width-(scrollBar.visible ? scrollBar.width : 0)
                    height: 20
                    color: "transparent"
                    Components.Label {
                        id:sec_l
                        text: section
                        font.weight:Font.Bold
                        anchors.verticalCenter: sect.verticalCenter
                    }
                }
            }
            Components.ScrollBar {
                id: scrollBar
                orientation: Qt.Vertical
                flickableItem:list
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
            }
        }
    }
}
