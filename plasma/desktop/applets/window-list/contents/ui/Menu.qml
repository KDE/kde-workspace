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
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: menu
    property alias model: menuListView.model
    property alias section: menuListView.section
    property int iconSize: theme.smallIconSize 
    property bool showDesktop: true
    signal itemSelected(string source)
    signal executeJob(string jobName, string source)
    signal setOnDesktop(string source, int desktop)
    
    Component {
        id:sectionheader
        Rectangle {
            width:menu.width
            height:30
            color:"transparent"
            border.width:5
            radius:10
            border.color:"transparent"
            PlasmaComponents.Highlight {
                hover:menu.focus
                width:menu.width
                height:30
            }
            PlasmaComponents.Label {
                text:section>0?i18n("Desktop %1 " ,section):i18n("On all desktops")
                anchors {
                    centerIn:parent
                }
            }
        }
    }

    ListView {
        id: menuListView
        width: menu.width
        anchors.top: menu.top
        anchors.left: menu.left
        anchors.bottom:menu.bottom
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        section.property:desktop
        section.criteria:ViewSection.FullString
        section.delegate: sectionheader
        delegate: TaskDelegate {
            id: menuItemDelegate
            width: menuListView.width
            property string source: DataEngineSource
            name: model["name"]
            desktop: model["desktop"]
            icon: model["icon"]
            active: model["active"]
            iconSize: menu.iconSize
            showDesktop: menu.showDesktop
            onClicked: menu.itemSelected(source);
            onEntered: menuListView.currentIndex = index; 
            onExecuteJob: menu.executeJob(jobName, source);
            onSetOnDesktop: menu.setOnDesktop(source, desktop);
        }
    }
}
