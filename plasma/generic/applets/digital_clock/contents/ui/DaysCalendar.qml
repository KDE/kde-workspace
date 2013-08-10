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
Row {
    id:calendarGrid
    width:calendarColumn.width
    height:(parent.height*9/10)
    property alias days:days
    Item {
        width: calendarGrid.width/8
        height: parent.height*0.9
        Column {
            width: calendarGrid.width/8
            height: parent.height*0.9
            spacing:0
            clip:true
            anchors.fill:parent
            anchors.topMargin:10
            Repeater { 
                model: monthCalendar.weeksModel
                width:(calendarGrid.width*7/8)/monthCalendar.days
                height:0.95*calendarGrid.height/monthCalendar.weeks
                Item {
                    width:(calendarGrid.width*7/8)/monthCalendar.days
                    height:0.95*calendarGrid.height/monthCalendar.weeks
                    Components.Label {
                        id:weekNumber
                        text: modelData + 1
                        opacity:0.5
                        horizontalAlignment: Text.AlignRight
                        anchors.centerIn:parent
                    }
                }
            }
        }
    }
    Grid {
        id:calendarDays
        columns:monthCalendar.days
        rows:1+monthCalendar.weeks
        spacing:0
        property Item selectedItem
        Repeater {
            id:days
            model: monthCalendar.days
            Item {
                width:calendarGrid.width*1/8
                height:10
                Components.Label {
                    text: Qt.formatDate(new Date(showDate.getFullYear(), showDate.getMonth(), index - firstDay +1), "ddd");
                    horizontalAlignment: Text.AlignHCenter
                    anchors.centerIn:parent
                }
            }
        }
        Repeater {
            id:repeater
            model:monthCalendar.model
            Rectangle {
                id:myRectangle
                width:(calendarGrid.width*7/8)/monthCalendar.days
                height:0.95*calendarGrid.height/monthCalendar.weeks
                color:(dateMouse.containsMouse)?"#eeeeee":"transparent"
                border.color:isToday(dayNumber+"/"+monthNumber+"/"+yearNumber)?"blue":calendarDays.selectedItem == myRectangle ? "black" : "transparent"
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
                        text=(containsEventItems)||(containsTodoItems)?"":eventDate(yearNumber,monthNumber,dayNumber)
                        text_event=(containsEventItems)||(containsTodoItems)?"":" No events found on this day ";
                        calendarDays.selectedItem=myRectangle
                    }
                }
            }
        }
    }
}