import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts
import org.kde.qtextracomponents 0.1 as QtExtra

Item {
    id: active_win  
    property string text
    property alias icon: currentApp.icon 
    property int iconHeight
    property int iconWidth
    signal currentAppWidgetClicked
    signal maximizeClicked
    signal unmaximizeClicked
    signal closeClicked
    state: "unmaximized"
    Row {
        id: row
        width: parent.width
        height: parent.height
        PlasmaWidgets.IconWidget {
            id: currentApp
            orientation: Qt.Horizontal
            maximumIconSize: "30x30"
            height: 500
            icon: active_win.icon
        }
        Window_3 {
            id: text_style
            width: parent.width
            text: active_win.text
        }
    }
    MouseArea {
        anchors.fill: row
        hoverEnabled: true
        onClicked: {
            active_win.currentAppWidgetClicked();
        }
    }
    PlasmaCore.SvgItem {
        id: minMaximizeIcon
        anchors.verticalCenter: row.verticalCenter
        height: active_win.iconHeight
        width: active_win.iconWidth
        x: row.width
        svg: winOperationSvg
        elementId: "maximize"
        Behavior on opacity {
            PropertyAnimation { duration: 100 }
        }
    }
    MouseArea {
        anchors.fill: minMaximizeIcon
        hoverEnabled: true
        onEntered: {
	    if (minMaximizeIcon.opacity == 1)
	        minMaximizeIcon.opacity = .4
        }
        onExited: {
            if (minMaximizeIcon.opacity != 0)
	        minMaximizeIcon.opacity = 1
        }
        onClicked: {
            if (active_win.state == "maximized")
	        active_win.unmaximizeClicked()
            if (active_win.state == "unmaximized") 
	        active_win.maximizeClicked()
        }
    }
    states: [
        State {
            name: "maximized"
            PropertyChanges {
	        target: minMaximizeIcon
	        elementId: "unmaximize"
	        opacity: 1
	        visible: true
            }
            PropertyChanges {
	        target: closeIcon
	        opacity: 1
        	visible: true
            }
        },
        State {
            name: "unmaximized"
            PropertyChanges {
	        target: minMaximizeIcon
	        elementId: "maximize"
	        opacity: 1
	        visible: true
            }
            PropertyChanges {
	        target: closeIcon
	        opacity: 1
	        visible: true
            }
        },
        State {
            name: "noWindow"
            PropertyChanges {
         	target: currentApp
	        icon: QIcon("")
            }
        },
        State {
            name: "newWindowAdded"
            PropertyChanges {
	        target: minMaximizeIcon
	        opacity: 1
	        visible: true
            }
            PropertyChanges {
	        target: closeIcon
	        opacity: 1
        	visible: true
            }
        }
    ]
  
    transitions: [
        Transition {
	    to: "newWindowAdded"
	    ParallelAnimation {
	        PropertyAction { target: minMaximizeIcon; property: "visible"; value: true; }
	        PropertyAction { target: closeIcon; property: "visible"; value: true; }
	    }
        },
        Transition {
            from: "newWindowAdded"
            SequentialAnimation {
             	NumberAnimation { target: currentApp;  property: "opacity"; from: 1; to: .3; duration: 500; }
	        NumberAnimation { target: currentApp;  property: "opacity"; from: .3; to: 1; duration: 500; }
            }
        }, 
        Transition {
            to: "noWindow"
            ParallelAnimation {
        	NumberAnimation { target: minMaximizeIcon;  property: "opacity"; from: 1; to: 0; duration: 500; }
	        NumberAnimation { target: closeIcon;  property: "opacity"; from: 1; to: 0; duration: 400; }
            }
        },
        Transition {
            from: "noWindow"
            ParallelAnimation {
	        NumberAnimation { target: minMaximizeIcon;  property: "opacity"; from: 0; to: 1; duration: 400; }
	        PropertyAction { target: minMaximizeIcon; property: "visible"; value: true; }
	        NumberAnimation { target: closeIcon;  property: "opacity"; from: 0; to: 1; duration: 400; }
	        PropertyAction { target: closeIcon; property: "visible"; value: true; }
            }
        }
    ]
    
    PlasmaCore.Svg {
        id: winOperationSvg
        imagePath: "widgets/configuration-icons"
    }
  
}