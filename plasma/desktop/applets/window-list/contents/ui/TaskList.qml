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
    height: childrenRect.height
    signal clicked()
    signal entered()
    signal executeJob(string jobName)
    signal setOnDesktop(int desktop)
    property alias name: label.text
    property int desktop
    property alias icon: iconItem.icon
    property bool active: true
    property bool minimized: false
    property bool maximized: false
    property bool shaded: false
    property bool alwaysOnTop: false
    property bool keptBelowOthers: false
    property bool fullScreen: false
    property int iconSize: theme.smallIconSize
    property bool showDesktop: true
    property variant desktopItems: []
    
    QtObject {
        id: internal
        function defineDesktopSubLabel() {
            if (showDesktop) {
                var desktopString = i18n("Desktops: ");
                desktopString += desktop <= 0 ? "all" : main.desktopList[desktop-1];
                subLabelDesktop.text = desktopString;
            }
        }
    }
    onDesktopChanged: internal.defineDesktopSubLabel();

    Row {
        id: row
        width: parent.width
        height: Math.max(iconItem.height, label.height )
        QIconItem {
            id: iconItem
            anchors.verticalCenter: row.verticalCenter
            width: menuItem.iconSize
            height: menuItem.iconSize
        }
        Rectangle {
            width : menuItem.width -  menuItem.iconMargin - iconItem.width
            height:20
            color:"transparent"
            anchors.left: iconItem.right
            anchors.leftMargin:iconItem.width
            anchors.verticalCenter: iconItem.verticalCenter
            PlasmaCore.FrameSvgItem {
                id: action_task
                imagePath:"widgets/viewitem"
                prefix:"selected+hover"
                width: windowListMenu.width
                height:20
                visible:false
            } 
            PlasmaComponents.Label {
                id: label
                font.weight: menuItem.active ? Font.Bold : Font.Normal
                font.italic: (minimized == true) ? true : false
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            action_task.visible=true
            menuItem.clicked();
        }
        onEntered:{
            action_task.visible=true
            menuItem.entered();
        }
        onExited: {
             action_task.visible=false
        }
    }
}
