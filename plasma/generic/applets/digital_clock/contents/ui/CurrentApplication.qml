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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    id: active_win
    signal currentAppWidgetClicked
    Row {
        id: row
        width: parent.width
        height: parent.height
        Rectangle {
            id:currentApp
            color:"transparent"
            width:main.width
            height:width
            smooth: true
              anchors {
                left:parent.left
                right:parent.right
                top:parent.top
                bottom:parent.bottom
                centerIn:parent
            }
            Components.Label  {
                id: time
                //That will be used in cased .ui file is used but in plasma2 ui will not be used so I have to look for alter means -->
                /*  font.family:textFont
                 *        font.bold: bold?true:false
                 *        style:  shadow?Text.Raised:Text.Normal
                 *        styleColor:shadowcolor? shadowColor: "transparent"
                 *        font.italic:italic?true:false
                 *        color: textColor*/
                font.pointSize: 10
                text : (Qt.formatTime( dataSource.data["Local"]["Time"],"hh:mmap" )).toString().slice(0,-2)
                anchors {
                    top: parent.top;
                    left: parent.left;
                }
            }
            Components.Label  {
                id: ampm
                /*  font.family:textFont
                 *        font.bold: bold?true:false
                 *         font.italic:italic?true:false
                 *           style:  shadow?Text.Outline:Text.Normal
                 *        styleColor:shadow? shadowColor: "transparent"*/
                // opacity: 0.5
                color: textColor
                text : Qt.formatTime( dataSource.data["Local"]["Time"],"ap" )
                anchors {
                    top: parent.top;
                    left: time.right;
                }
            }
            Components.Label  {
                id: date
                // font.family:textFont
                // font.bold: bold?true:false
                //  font.italic:italic?true:false
                //  style:  shadow?Text.Outline:Text.Normal
                //   styleColor:shadowcolor? shadowColor: "transparent"
                //  color: textColor
                // opacity: 0.5
                text : Qt.formatDate( dataSource.data["Local"]["Date"],"dddd, MMM dd" )
                anchors {
                    top: time.bottom;
                    left: parent.left;
                }
            }
        }
    }
    MouseArea {
        anchors.fill: row
        hoverEnabled: true
        onClicked: {
            active_win.currentAppWidgetClicked();
        }
    }
}