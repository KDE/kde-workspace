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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: menuItem
    width: parent.width
    height: Math.max(iconItem.height, label.height) + action_task_1.marginHints.top + action_task_1.marginHints.bottom
    anchors.top:col.bottom
    signal clicked()
    signal entered()
    signal executeJob(string jobName)
    signal setOnDesktop(int desktop)
    property alias name: label.text
    property int desktop
    property alias icon: iconItem.icon
    property bool active:false
    property int iconSize: theme.smallIconSize
    property bool showDesktop: true
    property variant desktopItems: []
   // property alias highlight:highlight
    
    PlasmaComponents.Highlight {
        hover:menu.focus
        width: windowListMenu.width
        height:30
        visible:true
        opacity:menuItem.active?1:0
    }
    
    QIconItem {
        id: iconItem
        x: action_task_1.marginHints.left
        width: menuItem.iconSize
        height: menuItem.iconSize
    }
    
    PlasmaComponents.Label {
        id: label
        clip:true
        font.italic: (minimized == true) ? true : false
        anchors.left: iconItem.right
        anchors.leftMargin: action_task_1.marginHints.left
    }
    
    MouseArea {
        id:root_item
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            menuItem.clicked();
        }
        onEntered:{
            action_task_1.y = mapToItem(main,mouse.x,mouse.y).y - action_task_1.marginHints.top
            action_task_1.width = menuItem.width
            action_task_1.height = menuItem.height
            menuItem.entered();
            highlight=true
            print(highlight);
        }
        onExited: {
            highlight=false
        }
    }
}

