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


Item {
    id: listWidget  
    property alias listView: task_list  
    property int itemSpacing: 0  
    height: task_list.height + padding
    width: task_list.width
    property int padding:0
    Column {
        id:col
        width:task_list.width
        
        Rectangle {
            width:400
            height:30
            color:"lightgrey"
            border.width:5
            radius:10
            border.color:"transparent"
            rotation: 360
            gradient: Gradient {
                GradientStop { position: 1.0; color: "lightgrey" }
                GradientStop { position: 0.0; color: "grey" }
            }
            Components.Label {
                id:actions
                text:"Actions"
                anchors {
                    centerIn:parent
                }
                horizontalAlignment:Text.AlignHCenter
            }
        }
        Rectangle {
            width:400
            height:30
            color:"transparent"
            Components.Label {
                id:unclutter
                text:"Unclutter Windows"
                anchors {
                    left:parent.left
                }
                horizontalAlignment:Text.AlignHCenter
            }
            MouseArea {
                id: mouse
                hoverEnabled: true
                onReleased: active_win.unmaximizeClicked()
                anchors.fill:parent
            }
        }
        Rectangle {
            width:400
            height:30
            color:"transparent"
            Components.Label {
                id:cascade
                text:"Cascade Windows"
                anchors {
                    left:parent.left
                }
                horizontalAlignment:Text.AlignHCenter
            }
            MouseArea {
                id: mouseArea
                hoverEnabled: true
                onReleased:active_win.maximizeClicked()
                anchors.fill:parent
            }
        }
        Rectangle {
            width:400
            height:30
            color:"lightgrey"
            border.width:5
            radius:10
            border.color:"transparent"
            rotation: 360
            gradient: Gradient {
                GradientStop { position: 1.0; color: "lightgrey" }
                GradientStop { position: 0.0; color: "grey" }
            }
            Components.Label {
                id:text_style
                text:"Desktop"
                anchors {
                    centerIn:parent
                }
                horizontalAlignment:Text.AlignHCenter
            }
        }
        ListView {
            id: task_list
            height: 400
            width: 400
            model: PlasmaCore.DataModel { dataSource: tasksSource }
            delegate: listDelegate
            highlightRangeMode: ListView.StrictlyEnforceRange
            highlight: Rectangle { color: "grey"; radius: 5 }
            focus: true
            clip: true
        }
    }
}

