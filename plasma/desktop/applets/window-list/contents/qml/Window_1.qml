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
import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts
import org.kde.qtextracomponents 0.1 as QtExtra

Item {
    id: activew
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
        width: parent.width - 32
        height: parent.height
        PlasmaWidgets.IconWidget {
            id: currentApp
            orientation: Qt.Horizontal
            maximumIconSize: "32x32"
            height: row.height
            icon: activew.icon
        }
        Window_3 {
            id: fontstyle
            width: parent.width -32
            fontSize: 9
            smooth: true
            text: activew.text
        }
    }
    MouseArea {
        anchors.fill: row
        hoverEnabled: true
        onEntered: {
            fontstyle.startMarquee();
        }
        onExited: {
            fontstyle.reset();
        }
        onClicked: {
            activew.currentAppWidgetClicked();
        }
    }
    PlasmaCore.SvgItem {
        id: maxi
        anchors.verticalCenter: row.verticalCenter
        height: activew.iconHeight
        width: activew.iconWidth
        x: row.width
        svg: popup
        elementId: "maximize"    
        Behavior on opacity {
            PropertyAnimation { duration: 100 }
        }
    }
    PlasmaCore.SvgItem {
        id: closeIcon
        anchors.verticalCenter: row.verticalCenter
        height: activew.iconHeight
        width: activew.iconWidth
        x: row.width + 16
        svg: popup
        elementId: "close"
    }
    MouseArea {
        anchors.fill: maxi
        hoverEnabled: true
        onEntered: {
            if (maxi.opacity == 1)
	        maxi.opacity = .4
        }
        onExited: {
            if (maxi.opacity != 0)
	        maxi.opacity = 1
        }
        onClicked: {
            if (activew.state == "maximized")
	        activew.unmaximizeClicked()
	    if (activew.state == "unmaximized") 
	        activew.maximizeClicked()
        }
    }
    MouseArea {
        anchors.fill: closeIcon
        hoverEnabled: true
        onEntered: {
        if (closeIcon.opacity == 1)
	    closeIcon.opacity = .4
        }
        onExited: {
            if (closeIcon.opacity != 0)
	        closeIcon.opacity = 1
        }
        onClicked: {
            activew.closeClicked()
        }
    }
    states: [
        State {
            name: "maximized"
            PropertyChanges {
	        target: maxi
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
	        target: maxi
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
	        target: maxi
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
	        PropertyAction { target: maxi; property: "visible"; value: true; }
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
	    NumberAnimation { target: maxi;  property: "opacity"; from: 1; to: 0; duration: 500; }
	    NumberAnimation { target: closeIcon;  property: "opacity"; from: 1; to: 0; duration: 400; }
            }
        },
        Transition {
            from: "noWindow"
            ParallelAnimation {
	        NumberAnimation { target: maxi;  property: "opacity"; from: 0; to: 1; duration: 400; }
	        PropertyAction { target: maxi; property: "visible"; value: true; }
	        NumberAnimation { target: closeIcon;  property: "opacity"; from: 0; to: 1; duration: 400; }
	        PropertyAction { target: closeIcon; property: "visible"; value: true; }
            }
        }
    ]
    PlasmaCore.Svg {
        id: popup
        imagePath: "widgets/configuration-icons"
    }
}