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

Component {
  
    Item {
    
        signal closeClicked(string wid)
        signal activateWindow(string wid)
        width: task_list.width
        height: 32
        clip: true
        state: "unminized"
        
    
        Row {
            id: taskRow
            width: dialog.width 
            
            PlasmaWidgets.IconWidget {
	        id: taskIconWidget
	        maximumIconSize: "32x32"
	        orientation: Qt.Horizontal
	        icon: main.data[DataEngineSource]['icon']
            }
            Window_3 {
                id: text_style
                width: parent.width
                smooth: true
                font.italic: { (minimized == true) ? true : false }	    text: name
            }
            Behavior on opacity {
                PropertyAnimation { duration: 300 }
            }
        }

        MouseArea {
            anchors.fill: taskRow
            hoverEnabled: true
            onEntered: {
	        if (!closeIcon.visible) {
	            closeIcon.visible = true
	        }
	    taskRow.opacity = 0.5
            }
            onExited: {
	        if (closeIcon.visible) {
	            closeIcon.visible = false
	        }
	        taskRow.opacity = 1
            }
            onClicked: {
                main.activateWindow(DataEngineSource);
            }
        }
    
    
        PlasmaCore.SvgItem {
            id: closeIcon
            anchors.verticalCenter: parent.verticalCenter
            x: taskRow.width
            width: main.closeIconSize
            height: main.closeIconSize
            svg: winOperationSvg
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
            id: winOperationSvg
            imagePath: "widgets/configuration-icons"
        }
    }
}