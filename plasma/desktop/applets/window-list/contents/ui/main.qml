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
    id:main
    property int minimumWidth:formFactor == Horizontal ? height : 1
    property int minimumHeight:formFactor == Vertical ? width  : 1
    property int formFactor: plasmoid.formFactor
    property bool constrained:formFactor==Vertical||formFactor==Horizontal
    property string activeWindowId: "";
    property string newWindowId: "";
    property bool activeWorkspace : false;
    property int closeIconSize: 10
    property int winOperationIconSize: 10
    property int appIconSize: 10
    property alias data: tasksSource.data;
    property alias connectedSources: tasksSource.connectedSources;
    signal activeWindowChanged(string wid);
    signal activeWindowMinimized;
    signal windowAdded(string wid);
    signal windowRemoved(string wid);
    signal windowStateChanged(string wid);
    signal workspaceActivated;
    function closeWindow(id) {
        if (!id)
            return;
        var source = tasksSource.serviceForSource(id);
        var data = source.operationDescription("close");
        source.startOperationCall(data);
    }
    function activateWindow(id) {
        var source = tasksSource.serviceForSource(id);
        var data = source.operationDescription("activate");
        source.startOperationCall(data);
    }
    function setWindowMaximized(id, state) {
        if (!id)
            return;
        var source = tasksSource.serviceForSource(id);
        var data = source.operationDescription("setMaximized");
        data['maximized'] = state;
        source.startOperationCall(data);
    }
    function isMaximized(id) {
        print(id);
        if (!id)
            return false;
        return tasksSource.data[id]['maximized'];
    }
    function isMinimized(id) {
        if (!id) 
            return false;
        return tasksSource.data[id]['minimized'];
    }
    function getPopupPosition(dialogContainer, object) {
        var location = plasmoid.locoation;
        var pos = dialogContainer.popupPosition(object);
        print("Pos " + pos.x + "," + pos.y);
        switch(location) {
            case Floating: 
            case TopEdge: {
                pos.y += object.height;
                break;
            }
            case BottomEdge: {
                pos.y -= object.height;
            }
            case LeftEdge: {
                pos.x += object.width;
            }
            case RightEdge: {
                pos.x -= object.width
            }
            case FullScreen: {
            }
            default: {
            }
        }
        return pos;
    }
    function makeTaskListVisible(visibility) {
        dialogContainer.visible = visibility;
        dialog.visible = visibility;
    }
    PlasmaCore.DataSource {
        id: tasksSource
        engine: "tasks"
        onSourceAdded: {
            connectSource(source);
            main.newWindowId = source;
            main.windowAdded(source);
            print("onSourceAdded: " + source)
        }
        onNewData: {
            if (data['minimized']) {
                if (main.activeWindowId == sourceName) {
                    main.activeWindowMinimized();
                    print("Minimizing " + sourceName)
                }
            }
            if (data['active']) {
                main.activeWorkspace = false;
                main.activeWindowId = sourceName;
                main.activeWindowChanged(sourceName);
            } else {
                workspaceTimer.start();
                main.activeWorkspace = true;
            }
        }
        onSourceRemoved: {
            main.windowRemoved(source);
        }
        Component.onCompleted: {
            connectedSources = sources;
            for (var key in sources){
                var s = sources[key];
                print(s + " added");
                main.windowAdded(s);
            }
        }
    }
    CurrentApplication {
        id:active_win
        width: main.width
        height:width
        PlasmaCore.IconItem  {
            width:main.width
            height:width
            smooth: true
            source: "preferences-system-windows.png"
            anchors {
                left:parent.left
                right:parent.right
                top:parent.top
                bottom:parent.bottom
                centerIn:parent
            }
        }
    }
    TaskRow {
        id: listDelegate
    }
    TaskList { 
        id: dialog 
    }
    Timer {
        id: workspaceTimer
        interval: 100
        onTriggered: {
            if (main.activeWorkspace)
                main.workspaceActivated();
            else
                workspaceTimer.stop();
        }
    }
    PlasmaCore.Dialog {
        id: dialogContainer
        visible: false
        mainItem: dialog
        Component.onCompleted: {
            setAttribute(Qt.WA_X11NetWmWindowTypeDock, true);
        }
    }
    Connections {
        target: plasmoid
        onFormFactorChanged: {
            main.formFactor = plasmoid.formFactor
            if(main.formFactor==Planar || main.formFactor == MediaCenter )
            {
                minimumWidth=main.width/3.5
                minimumHeight=main.height/3.5
            }
        }
        onActiveWindowChanged: {
            if (main.newWindowId == wid)
                active_win.state = "newWindowAdded";
            active_win.state = main.isMaximized(wid) ? "maximized" : "unmaximized"
            main.newWindowId = "";
            if (dialog.visible)
                main.makeTaskListVisible(false);
        }
        onWindowAdded: {
            var size = main.appIconSize + dialog.itemSpacing;
            dialog.listView.spacing=0
        }
        onWindowRemoved: {
                var size = main.appIconSize + dialog.itemSpacing;
                dialog.listView.spacing=0
        }
        onWorkspaceActivated: {
            if (dialog.visible)
                main.makeTaskListVisible(false);
            main.activeWindowId = ""
            active_win.setIcon("preferences-system-windows")
            active_win.setBackgroundHints(0)
            active_win.state = "noWindow"
        }
    }
    PlasmaCore.ToolTip {
        target: mouseArea
        mainText:"Window list"
        subText:"Show list of opened windows"
        image:"preferences-system-windows"
    }
    Connections {
        target: active_win
        onCurrentAppWidgetClicked: {
            var pos = main.getPopupPosition(dialogContainer, active_win);
            dialogContainer.x = pos.x;
            dialogContainer.y = pos.y;
            if (dialogContainer.visible)
                main.makeTaskListVisible(false);
            else
                main.makeTaskListVisible(true);
        }
        onMaximizeClicked: {
            main.setWindowMaximized(main.activeWindowId, true);
        }
        onUnmaximizeClicked: {
            main.setWindowMaximized(main.activeWindowId, false);
        }
        onCloseClicked: {
            main.closeWindow(main.activeWindowId);
        }
    }
}
