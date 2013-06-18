import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts
import org.kde.qtextracomponents 0.1 as QtExtra
QGraphicsWidget {
  id: mainWidget
  preferredSize: "40x16"
  minimumSize: "40x16"
    Item {
        id:main
        width: mainWidget.width
        height: mainWidget.height 
        property string activeWindowId: "";
        property string newWindowId: "";
        property bool activeWorkspace : false;
        property int closeIconSize: 16
        property int winsize: 16
        property int appIconSize: 32
        property alias data: tasksSource.data;
        property alias connectedSources: tasksSource.connectedSources;    
        signal activeWindowChanged(string wid);
        signal activeWindowMinimized;    
        signal windowAdded(string wid);
        signal windowRemoved(string wid);
        signal windowStateChanged(string wid);
        signal workspaceActivated;    
        function close(id) {
        if (!id)
	    return;
            var source = tasksSource.serviceForSource(id);
            var data = source.operationDescription("close");
            source.startOperationCall(data);
        }
        function activate(id) {
            var source = tasksSource.serviceForSource(id);
            var data = source.operationDescription("activate");
            source.startOperationCall(data);
        }
        function maximizew(id, state) {
            if (!id)
	        return;
            var source = tasksSource.serviceForSource(id);
            var data = source.operationDescription("setMaximized");
            data['maximized'] = state;
            source.startOperationCall(data);
        }
        function ifmaximize(id) {
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
        function dialog_pos(dialogContainer, object) {
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
        function displayw(visibility) {
            dialogContainer.visible = visibility;
            dialog.visible = visibility;
        }
        function adjustw(dy) {
            dialogContainer.height += dy;
            dialog.height += dy;
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
        Window_1 {
            id: activew
            iconHeight: 10
            iconWidth: 10
            height: parent.height
            width: parent.width
            Rectangle{
                width: activew.width; height:activew.height
                color:"transparent"
                Image  {
                    width: activew.width; height:activew.height
                    smooth: true
                    source: "/usr/share/icons/oxygen/128x128/apps/preferences-system-windows.png"
                }
            }
        }
        Window_4 {
            id: listDelegate 
        }
        Window_2 {
            id: dialog
            Item {
                id: name
                height: 20
                width: parent.width  
                Rectangle {
                    id:rect
                    width:300
                    height:20
                    border.color:"silver"
                    border.width:1
                    radius:7
                    color:"silver"
                    Text {
                        id: text
                        font.pixelSize: 15
                        color: "black"  
                        text: "Desktop" + "\n" 
                        style:Text.Outline;styleColor:"gray"
                    }
                }
            }
        }
        Timer {
            id: workspaceTimer
            interval: 100
            onTriggered: {
	        if (main.activeWorkspace) {
	            main.workspaceActivated();
                }
	        else {
	            workspaceTimer.stop(); 
                }
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
            onActiveWindowChanged: {
	        print(activew.state)
		if (main.newWindowId == wid)
	            activew.state = "newWindowAdded";
		print(activew.state)
	        activew.state = main.ifmaximize(wid) ? "maximized" : "unmaximized"
		print(activew.state)
	        activew.text = tasksSource.data[wid]['name'];
	        activew.icon = tasksSource.data[wid]['icon'];
		main.newWindowId = "";
		if (dialog.visible)
	            main.displayw(false);
            }
            onWindowAdded: {
	        print("Adding window " + wid)
	        var size = main.appIconSize + dialog.itemSpacing;
		dialog.listView.anchors.top=name.bottom;
	        dialog.listView.spacing=0
	        dialog.listView.height += size
            }
            onWindowRemoved: {
	        print("Removing window " + wid)
		var size = main.appIconSize + dialog.itemSpacing;
	        dialog.listView.height -= size
            }
            onWorkspaceActivated: {
	        if (dialog.visible)
	            main.displayw(false);
	  	main.activeWindowId = ""
	        activew.state = "noWindow"
                activew.setIcon("preferences-system-windows")
                activew.setBackgroundHints(0)
            }
        }
        Connections {
            target: activew
            onCurrentAppWidgetClicked: {
	        var pos = main.dialog_pos(dialogContainer, activew);
	        dialogContainer.x = pos.x;
	        dialogContainer.y = pos.y;
        	if (dialogContainer.visible)
	            main.displayw(false);
	        else
	            main.displayw(true);
            }
            onMaximizeClicked: {
	        main.maximizew(main.activeWindowId, true);
	        print("Maximizing " + main.activeWindowId);
            }
            onUnmaximizeClicked: {
	        main.maximizew(main.activeWindowId, false);
            }
            onCloseClicked: {
	        main.close(main.activeWindowId);
            }
        }
  }
}
