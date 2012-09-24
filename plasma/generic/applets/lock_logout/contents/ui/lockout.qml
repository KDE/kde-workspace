/*
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1
import org.kde.qtextracomponents 0.1
import "data.js" as Data

Flow {
    id: lockout
    property int minimumWidth
    property int minimumHeight

    property int minButtonSize: 16

    property bool show_lock: true
    property bool show_switchUser: false
    property bool show_leave: true
    property bool show_suspend: false
    property bool show_hibernate: false
    property int visibleButtons: 2
    property int orientation: Qt.Vertical

    flow: orientation==Qt.Vertical ? Flow.TopToBottom : Flow.LeftToRight

    onWidthChanged: checkLayout();
    onHeightChanged: checkLayout();

    PlasmaCore.DataSource {
        id: dataEngine
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
    }

    Component.onCompleted: {
        plasmoid.aspectRatioMode = IgnoreAspectRatio;
        plasmoid.addEventListener('ConfigChanged', configChanged);
    }

    function checkLayout() {
        switch(plasmoid.formFactor) {
        case Vertical:
            if (width >= minButtonSize*visibleButtons) {
                orientation = Qt.Horizontal;
                minimumHeight = width/visibleButtons - 1;
                plasmoid.setPreferredSize(width, width/visibleButtons);
            } else {
                orientation = Qt.Vertical;
                minimumHeight = width*visibleButtons;
                plasmoid.setPreferredSize(width, width*visibleButtons);
            }
            break;

        case Horizontal:
            if (height < minButtonSize*visibleButtons) {
                orientation = Qt.Horizontal;
                minimumWidth = height*visibleButtons;
                plasmoid.setPreferredSize(height*visibleButtons, height);
            } else {
                orientation = Qt.Vertical;
                minimumWidth = height/visibleButtons - 1;
                plasmoid.setPreferredSize(height/visibleButtons, height);
            }
            break;

        default:
            if (width > height) {
                orientation = Qt.Horizontal;
                minimumWidth = minButtonSize*visibleButtons;
                minimumHeight = minButtonSize;
            } else {
                orientation = Qt.Vertical;
                minimumWidth = minButtonSize;
                minimumHeight = minButtonSize*visibleButtons;
            }
            break;
        }
    }

    function configChanged() {
        show_lock = plasmoid.readConfig("show_lock");
        show_switchUser = plasmoid.readConfig("show_switchUser");
        show_leave = plasmoid.readConfig("show_leave");
        show_suspend = plasmoid.readConfig("show_suspend");
        show_hibernate = plasmoid.readConfig("show_hibernate");

        visibleButtons = show_lock+show_switchUser+show_leave+show_suspend+show_hibernate;

        showModel.get(0).show = show_lock;
        showModel.get(1).show = show_switchUser;
        showModel.get(2).show = show_leave;
        showModel.get(3).show = show_suspend;
        showModel.get(4).show = show_hibernate;

        checkLayout();
    }

    // model for setting whether an icon is shown
    // this cannot be put in data.js because the the variables need to be
    // notifiable for delegates to instantly respond to config changes
    ListModel {
        id: showModel
        // defaults:
        ListElement { show: true } // lock
        ListElement { show: false} // switch user
        ListElement { show: true } // leave
        ListElement { show: false} // suspend
        ListElement { show: false} // hibernate
    }

    Repeater {
        id: items
        property int itemWidth: parent.flow==Flow.LeftToRight ? Math.floor(parent.width/visibleButtons) : parent.width
        property int itemHeight: parent.flow==Flow.TopToBottom ? Math.floor(parent.height/visibleButtons) : parent.height
        property int iconSize: Math.min(itemWidth, itemHeight)

        model: Data.data

        delegate: Item {
            id: iconDelegate
            visible: showModel.get(index).show
            width: items.itemWidth
            height: items.itemHeight

            
            QIconItem {
                id: iconButton
                width: items.iconSize
                height: items.iconSize
                anchors.centerIn: parent
                icon: QIcon(modelData.icon)
                scale: mouseArea.pressed ? 0.9 : 1
                
                QIconItem {
                    id: activeIcon
                    opacity: mouseArea.containsMouse ? 1 : 0
                    anchors.fill: iconButton
                    icon: QIcon(modelData.icon)
                    state: QIconItem.ActiveState
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onReleased: clickHandler(modelData.operation)

                    PlasmaCore.ToolTip {
                        target: mouseArea
                        mainText: modelData.tooltip_mainText
                        subText: modelData.tooltip_subText
                        image: modelData.icon
                    }
                }
            } 
        }
    }

    Component {
        id: hibernateDialogComponent
        QueryDialog {
            titleIcon: "system-suspend-hibernate"
            titleText: i18n("Hibernate")
            message: i18n("Do you want to suspend to disk (hibernate)?")

            acceptButtonText: i18n("Yes")
            rejectButtonText: i18n("No")

            onAccepted: performOperation("suspendToDisk")
        }
    }
    property QueryDialog hibernateDialog

    Component {
        id: sleepDialogComponent
        QueryDialog {
            titleIcon: "system-suspend"
            titleText: i18n("Suspend")
            message: i18n("Do you want to suspend to RAM (sleep)?")

            acceptButtonText: i18n("Yes")
            rejectButtonText: i18n("No")

            onAccepted: performOperation("suspendToRam")
        }
    }
    property QueryDialog sleepDialog

    function clickHandler(what) {
        if (what == "suspendToDisk") {
            if (!hibernateDialog) {
                hibernateDialog = hibernateDialogComponent.createObject(lockout);
            }

            hibernateDialog.open();

        } else if (what == "suspendToRam") {
            if (!sleepDialog) {
                sleepDialog = sleepDialogComponent.createObject(lockout);
            }

            sleepDialog.open();

        } else {
            performOperation(what);
        }
    }

    function performOperation(what) {
        var service = dataEngine.serviceForSource("PowerDevil");
        var operation = service.operationDescription(what);
        service.startOperationCall(operation);
    }
}

