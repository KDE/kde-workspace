import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts
Component {
    Item {
        signal closeClicked(string wid)
        signal activateWindow(string wid)
        width: tasks.width
        height: 32
        clip: true
        state: "unminized"
    }
    Row {
        id: deskrow
        spacing: 3
        width: dialog.width - 32
        PlasmaWidgets.IconWidget {
	    id: listwidget
	    maximumIconSize: "32x32"
	    orientation: Qt.Horizontal
	    icon: main.data[DataEngineSource]['icon']
        }
        Window_3 {
	    id: fontstyle
	    anchors.verticalCenter: parent.verticalCenter
	    width: parent.width - 32 - 10
	    smooth: true
	    font.italic: { (minimized == true) ? true : false }
	    text: name
            Behavior on opacity {
        	PropertyAnimation { duration: 300 }
            }
        }
        MouseArea {
            anchors.fill: deskrow
            hoverEnabled: true
            onEntered: {
	        if (!closeIcon.visible) {
	            closeIcon.visible = true
	        }
	    deskrow.opacity = 0.5
	    print("Over" + name);
	    fontstyle.startMarquee();
            }
            onExited: {
        	if (closeIcon.visible) {
	            closeIcon.visible = false
	        }
	    fontstyle.reset();
	    deskrow.opacity = 1
            }
            onClicked: {
	        main.activateWindow(DataEngineSource);
            }
        }
        PlasmaCore.SvgItem {
            id: closeIcon
            anchors.verticalCenter: parent.verticalCenter
            x: deskrow.width
            width: main.closeIconSize
            height: main.closeIconSize
            svg: popup
            elementId: "close"
            visible: false
            Behavior on opacity {
	        PropertyAnimation { duration: 300 }
            }
        }
        MouseArea {
            anchors.fill: closeIcon
            hoverEnabled: true
            onEntered: {
	        closeIcon.opacity = 0.5
	        closeIcon.visible = true;
            }
            onExited: {
	        closeIcon.opacity = 1
	        closeIcon.visible = false;
            }
            onClicked: {
	        main.closeWindow(DataEngineSource);
            }
        }
        PlasmaCore.Svg {
        id: popup
        imagePath: "widgets/configuration-icons"
        }
    }
}