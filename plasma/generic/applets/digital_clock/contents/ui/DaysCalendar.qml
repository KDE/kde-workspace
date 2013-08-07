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
    id:grid
    width:col.width
    height:parent.height-rect1.height-riw.height
    Column {
        width: grid.width/8
        height: parent.height
        spacing:grid.width/12
        clip:true
        focus:true
        Repeater { 
            model: monthCalendar.weeksModel
            Components.Label {
                id:weekNumber
                text: modelData + 1
                opacity:0.5
            }
        }
    }
    Grid {
        id:gv
        columns:monthCalendar.days
        rows:1+monthCalendar.weeks//dayLabels.rows
        width:grid.width*7/8
        height:parent.height
        spacing:0
        property Item selectedItem
        Repeater {
            id:repeater
            model:monthCalendar.model
            Rectangle {
                id:myRectangle
                width:(grid.width*7/8)/monthCalendar.days
                height:grid.height/monthCalendar.weeks
                color:(dateMouse.containsMouse)?"#eeeeee":"transparent"
                border.color:isToday(dayNumber+"/"+monthNumber+"/"+yearNumber)?"blue":gv.selectedItem == myRectangle ? "black" : "transparent"
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
                        // error.text=(containsEventItems)||(containsTodoItems)?"":eventDate(yearNumber,monthNumber,dayNumber)
                        //  errorl.text=(containsEventItems)||(containsTodoItems)?"":" No events found on this day ";
                        gv.selectedItem=myRectangle
                    }
                }
            }
        }
    }
}