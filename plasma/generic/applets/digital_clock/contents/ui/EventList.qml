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
Column {
    id:eventColumn
    height: parent.height
    width: parent.width / 2
    /*property alias text: error.text
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
    }*/
    PlasmaExtras.ScrollArea {
        anchors.fill:parent
        ListView {
            id:list
            height: parent.height
            width: (parent.width / 4)
            model: monthCalendar.upcomingEventsModel
            delegate:Row {
                spacing:4
                width: parent.width
                height:50
                Components.Label {
                    id:sum
                    text:  mimeType.split('.')[mimeType.split('.').length - 1]  + ": \n"+ summary 
                    font.capitalization:Font.Capitalize
                    font.italic:true
                }
            }
            section.property:"startDate"
            section.delegate:Components.Label {
                id:sec_l
                text: section
                font.weight:Font.Bold
                anchors.verticalCenter: sect.verticalCenter
            }
        }
    }
}