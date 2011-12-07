/*
 *   Author: Viranch Mehta <viranch.mehta@gmail.com>
 *   Date: Mon Dec 7 2011
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

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
import "logic.js" as Logic

Item {
    id: calendar
    property int firstDayOfMonth
    property int year
    property int month
    property int day
    property date today
    width: (cellWidth*8+cellSpacing*7+10)*2
    height: 250

    Components.Menu {
        id: monthMenu
        Components.MenuItem { text: "January";   onTriggered: month=1; }
        Components.MenuItem { text: "February";  onTriggered: month=2; }
        Components.MenuItem { text: "March";     onTriggered: month=3; }
        Components.MenuItem { text: "April";     onTriggered: month=4; }
        Components.MenuItem { text: "May";       onTriggered: month=5; }
        Components.MenuItem { text: "June";      onTriggered: month=6; }
        Components.MenuItem { text: "July";      onTriggered: month=7; }
        Components.MenuItem { text: "August";    onTriggered: month=8; }
        Components.MenuItem { text: "September"; onTriggered: month=9; }
        Components.MenuItem { text: "October";   onTriggered: month=10; }
        Components.MenuItem { text: "November";  onTriggered: month=11; }
        Components.MenuItem { text: "December";  onTriggered: month=12; }
    }

    Row {
        id: headerBtns
        anchors {
            top: parent.top
            topMargin: 10
            left: parent.left
            leftMargin: 10
            right: table.right
        }
        
        Components.ToolButton {
            id: prevBtn
            text: "<"
            width: 30
            onClicked: {
                if (month==1) {
                    year-=1;
                    month=12;
                } else {
                    month-=1;
                }
            }
        }

        Components.ToolButton {
            id: monthBtn
            text: Logic.months[month-1]
            width: 100
            onClicked: {
                monthMenu.open();
            }
        }

        Components.ToolButton {
            id: yearBtn
            text: year.toString()
            width: 50
            onClicked: {
                visible=!visible;
                yearEdit.selectAll();
            }
        }

        Components.TextField {
            id: yearEdit
            text: yearBtn.text
            width: yearBtn.width
            visible: !yearBtn.visible
            Keys.onReturnPressed: {
                year = text.toNumber();
                yearBtn.visible = true;
            }
        }

        Components.ToolButton {
            id: nextBtn
            text: ">"
            width: 30
            onClicked: {
                month = month%12 + 1;
                if (month==1) year += 1;
            }
        }
    }

    PlasmaCore.Svg {
        id: calSvg
        imagePath: "widgets/calendar"
    }

    PlasmaCore.SvgItem {
        id: weekDayHeader
        svg: calSvg
        elementId: "weekDayHeader"
        anchors {
            top: headerBtns.bottom
            topMargin: 10
            left: table.left
        }
        width: table.width
        height: cellHeight
    }

    Row {
        id: weekdays
        anchors.fill: weekDayHeader
        spacing: 2
        property int startsAt: 0
        property int fontSize: 14
        Repeater {
            model: 7
            Text {
                text: Logic.weekdays[(weekdays.startsAt+index)%7]
                width: cellWidth
                height: cellHeight
                font.pixelSize: weekdays.fontSize
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    PlasmaCore.SvgItem {
        id: weeksColumn
        svg: calSvg
        elementId: "weeksColumn"
        anchors {
            top: weekDayHeader.bottom
            topMargin: 2
            left: headerBtns.left
        }
        width: cellWidth
        height: table.height
    }

    PlasmaCore.SvgItem {
        id: selectBorder
        svg: calSvg
        elementId: "selected"
        visible: false
        width: cellWidth+cellSpacing
        height: cellHeight+cellSpacing
    }

    PlasmaCore.SvgItem {
        id: todayBorder
        svg: calSvg
        elementId: "today"
        visible: Logic.getYear(today) == year && Logic.getMonth(today)==month
        x: cellXFromIndex(firstDayOfMonth+Logic.getDate(today)-1)
        y: cellYFromIndex(firstDayOfMonth+Logic.getDate(today)-1)
        width: cellWidth+cellSpacing
        height: cellHeight+cellSpacing
    }

    property int cellWidth: calSvg.elementSize("active").width*1.3
    property int cellHeight: calSvg.elementSize("active").height*1.3
    property int cellSpacing: 2

    function cellXFromIndex(index) {
        var x = (index%7)*(cellWidth+cellSpacing) - cellSpacing/2;
        return table.x+x;
    }
    function cellYFromIndex(index) {
        var y = Math.floor(index/7)*(cellHeight+cellSpacing) - cellSpacing/2;
        return table.y+y;
    }

    Grid {
        id: table
        anchors {
            top: weekDayHeader.bottom
            topMargin: 2
            left: weeksColumn.right
            leftMargin: 2
        }
        columns: 7
        spacing: cellSpacing
        Repeater {
            id: repeater
            model: 42
            Cell {
                dday: index-firstDayOfMonth+1;
                calendarSvg: calSvg
                width: cellWidth
                height: cellHeight
                onSelected: {
                    selectBorder.x = cellXFromIndex(index);
                    selectBorder.y = cellYFromIndex(index);
                    selectBorder.visible = true;
                }
            }
        }
    }

    PlasmaCore.Svg {
        id: lineSvg
        imagePath: "widgets/line"
    }

    PlasmaCore.SvgItem {
        id: separator
        svg: lineSvg
        elementId: "vertical-line"
        anchors {
            top: headerBtns.top
            bottom: parent.bottom
            bottomMargin: 10
            left: table.right
            leftMargin: 10
        }
        width: lineSvg.elementSize("vertical-line").width
    }
}
