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
import org.kde.locale 0.1

Item {
    id:main
    property int minimumWidth:formFactor == Horizontal ? height : 1
    property int minimumHeight:formFactor == Vertical ? width  : 1
    property int formFactor: plasmoid.formFactor
    property bool constrained:formFactor==Vertical||formFactor==Horizontal

      function setTimeFormat()
    {
        timeFormat = plasmoid.readConfig( "timeFormat" )
        if( timeFormat == 12 ){
            timeString = (Qt.formatTime( dataSource.data["Local"]["Time"],"h:mmap" )).toString().slice(0,-2)
        } else {
            timeString = (Qt.formatTime( dataSource.data["Local"]["Time"],"hh:mm" ))
        }
    }

   function getPopupPosition(dialogContainer, object) {
        var location = plasmoid.locoation;
        var pos = dialogContainer.popupPosition(object);
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
    
   function makeCalendarVisible(visibility) {
        dialogContainer.visible = visibility;
        dialog.visible = visibility;
    }
        PlasmaCore.DataSource {
        id: dataSource
        engine: "time"
        connectedSources: ["Local"]
        interval: 500
    }
    
    Locale {
        id: locale
    }
    
    CurrentApplication {
        id:active_win
        width: main.width
        height:width
    }
    
    Calendar { 
        id: dialog 
    }
    
    PlasmaCore.Dialog {
        id: dialogContainer
        visible: false
        mainItem: dialog
        Component.onCompleted: {
            plasmoid.setMinimumSize(100, 100)
            plasmoid.popupIcon = "preferences-system-time .png"
            plasmoid.setBackgroundHints( 0 )
            plasmoid.aspectRatioMode = "IgnoreAspectRatio"
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
    }
    
    PlasmaCore.ToolTip {
        target: active_win
        mainText:"Digital Clock"
        subText:"Shows digital time"
        image:"preferences-system-time"
    }
    
    Connections {
        target: active_win
       onCurrentAppWidgetClicked: {
            var pos = main.getPopupPosition(dialogContainer, active_win);
            dialogContainer.x = pos.x;
            dialogContainer.y = pos.y;
            if (dialogContainer.visible) {
                main.makeCalendarVisible(false);
            }
            else {
                main.makeCalendarVisible(true);
            }
        }
    }
}
