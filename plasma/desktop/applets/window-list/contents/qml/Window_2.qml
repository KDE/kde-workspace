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
    property int padding:10
    ListView {
        id: task_list
        height: 200
        width: 300
        anchors.fill: parent
        model: PlasmaCore.DataModel { dataSource: tasksSource }
        delegate: listDelegate
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlight: Rectangle { color: "grey"; radius: 5 }
        focus: true
        clip: true
       
      /* Components.Label {
        id:text_style
        text:"Desktop"
        anchors {
            bottom:model.top
            left:parent.left
            top:parent.top
            right:parent.right
        }
        */
    }
}

