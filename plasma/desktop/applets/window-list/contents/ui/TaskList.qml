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
    property variant activities: []
    property int desktop
    property alias icon: iconItem.icon
    property bool active: false
    property bool minimized: false
    property bool maximized: false
    property bool shaded: false
    property bool alwaysOnTop: false
    property bool keptBelowOthers: false
    property bool fullScreen: false
    property int iconSize: theme.smallMediumIconSize
    property int iconMargin: 6
    property bool showDesktop: true
    property variant desktopItems: []
    QtObject {
        id: internal
        function defineDesktopSubLabel() {
            if (showDesktop) {
                var desktopString = i18n("Desktops:");
                desktopString += desktop <= 0 ? "all" : main.desktopList[desktop-1];
                subLabelDesktop.text = desktopString;
            }
        }
    }
    onDesktopChanged: internal.defineDesktopSubLabel();
    TaskRow {
        id: contextMenu
        desktop: menuItem.desktop
        minimized: menuItem.minimized
        maximized: menuItem.maximized
        shaded: menuItem.shaded
        alwaysOnTop: menuItem.alwaysOnTop
        keptBelowOthers: menuItem.keptBelowOthers
        fullScreen: menuItem.fullScreen
        onExecuteJob: menuItem.executeJob(jobName);
        onSetOnDesktop: menuItem.setOnDesktop(desktop);
    }
    Item {
        id: row
        width: parent.width
        height: Math.max(iconItem.height, label.height ) + 4 * menuItem.iconMargin
        QIconItem {
            id: iconItem
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: menuItem.iconMargin
            width: menuItem.iconSize
            height: menuItem.iconSize
        }
        Column {
            id: column
            anchors.left: iconItem.right
            anchors.right: parent.right
            anchors.verticalCenter: iconItem.verticalCenter
            anchors.leftMargin: menuItem.iconMargin
            PlasmaComponents.Label {
                id: label
                width: menuItem.width -  menuItem.iconMargin - iconItem.width
                height: theme.defaultFont.mSize.height
                elide: Text.ElideMiddle
                font.weight: menuItem.active ? Font.Bold : Font.Normal
                font.italic: { (minimized == true) ? true : false }
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if (mouse.button == Qt.LeftButton) {
                menuItem.clicked();
            }
            else if (mouse.button == Qt.RightButton) {
                    contextMenu.populate();
                    var mapPos = menuItem.mapToItem(menuItem, mouse.x, mouse.y);
                    contextMenu.open(mapPos.x, mapPos.y);
                }
            }
        onEntered: menuItem.entered();
    }
}
