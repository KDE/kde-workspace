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
    id: main
    property int minimumWidth: Math.max(200, windowListMenu.implicitWidth) 
    property int minimumHeight:  Math.max(400, windowListMenu.implicitHeight) 
    property int maximumWidth
    property int maximumHeight
    property alias data: tasksSource.data;
    property variant id 
    property variant desktopList: []
    property int iconSize: theme.smallMediumIconSize 
    property int defaultMargin:0
    property bool showDesktop: true
    
    function performOperation(op) {
        print("id =" +id);
        var service =tasksSource.serviceForSource(id);
        var operation = service.operationDescription(op);
        service.startOperationCall(operation);
    }
    
    function executeJob(operationName, source) {
        var service = tasksSource.serviceForSource(source);
        var operation = service.operationDescription(operationName);
        service.startOperationCall(operation);
    }
    
    function setOnDesktop(source, desktop) {
        var service = tasksSource.serviceForSource(source);
        var operation = service.operationDescription("toDesktop");
        operation.desktop = desktop;
        service.startOperationCall(operation);
    }
    Component.onCompleted: {
            var toolTipData = new Object;
            toolTipData["image"] = "preferences-system-window"; 
            toolTipData["mainText"] = i18n("Window List"); 
            toolTipData["subText"] = i18n("Show list of opened windows");
            plasmoid.popupIconToolTip = toolTipData;
            plasmoid.popupIcon = QIcon("preferences-system-windows"); 
            plasmoid.aspectRatioMode = IgnoreAspectRatio;
    }
    PlasmaCore.DataSource {
        id: tasksSource
        engine: "tasks"
        onSourceAdded: {
            connectSource(source);
            id = source ;
        }
        Component.onCompleted: {
            connectedSources = sources;
            connectSource("virtualDesktops");
            main.desktopList = tasksSource.data["virtualDesktops"]["names"];
        }
    }
    PlasmaCore.SortFilterModel {
        id: tasksModelSortedByDesktop
        sortRole: "desktop"
        sourceModel: tasksModel
    }
    PlasmaCore.SortFilterModel { 
        id: tasksModel
        sourceModel: PlasmaCore.DataModel {
            dataSource: tasksSource
        }
    }
    Menu {
        id: windowListMenu
        anchors.top:parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: main.defaultMargin
        clip: true;
        model: tasksModelSortedByDesktop
        section.property: "desktop"
        section.criteria: ViewSection.FullString
        iconSize: main.iconSize
        showDesktop: main.showDesktop
        onItemSelected: main.executeJob("activate", source);
        onExecuteJob: main.executeJob(jobName, source);
        onSetOnDesktop: main.setOnDesktop(source, desktop);
        Column {
            id:col
            spacing:0
            Rectangle {
                width: windowListMenu.width
                height:20
                color:"lightgrey"
                PlasmaCore.FrameSvgItem {
                    id:background2
                    imagePath:"widgets/viewitem"
                    prefix:"selected+hover"
                    width: windowListMenu.width
                    height:20
                }
                border.width:5
                radius:10
                border.color:"transparent"
                PlasmaComponents.Label {
                    id:actions
                    text:"Actions"
                    font.weight:Font.Bold
                    anchors {
                        centerIn:parent
                    }
                }
            }
            Rectangle {
                width: windowListMenu.width
                height:20
                color:"transparent"
                PlasmaComponents.Label {
                    id:unclutter
                    text:"Unclutter Windows"
                    anchors {
                        left:parent.left
                    }
                }
                MouseArea {
                    id: mouse
                    hoverEnabled: true
                    anchors.fill:parent
                    onClicked: {
                        performOperation("unclutter");
                    }
                    onEntered: {
                        unclutter.opacity = 0.5
                    }
                    onExited: {
                        unclutter.opacity = 1
                    }
                }
            }
            Rectangle {
                width: windowListMenu.width
                height:20
                color:"transparent"
                PlasmaComponents.Label {
                    id:cascade
                    text:"Cascade Windows"
                    anchors {
                        left:parent.left
                    }
                }
                MouseArea {
                    id: mouseArea
                    hoverEnabled: true
                    anchors.fill:parent
                    onClicked: { 
                        performOperation("cascade");
                    }
                    onEntered: {
                        cascade.opacity = 0.5
                    }
                    onExited: {
                        cascade.opacity = 1
                    }
                }
            }
            Rectangle {
                id:rect1
                width: windowListMenu.width
                height:20
                color:"lightgrey"
                anchors.bottom:row.top
                PlasmaCore.FrameSvgItem {
                    id:background1
                    imagePath:"widgets/viewitem"
                    prefix:"selected+hover"
                    width: windowListMenu.width
                    height:20
                }
                border.width:5
                radius:10
                border.color:"transparent"
                PlasmaComponents.Label {
                    id: subLabelDesktop
                    visible: showDesktop
                    font.weight:Font.Bold
                    anchors {
                        centerIn:parent
                    }
                }
            }
        }
    }
}
