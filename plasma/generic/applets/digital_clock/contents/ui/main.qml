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
     property int minimumWidth:formFactor == Horizontal ? height : 1
    property int minimumHeight:formFactor == Vertical ? width  : 1
    property int formFactor: plasmoid.formFactor
   // property int minimumWidth:height
    //property int minimumHeight:width
    
    PlasmaCore.DataSource {
        id: dataSource
        engine: "time"
        connectedSources: ["Local"]
        interval: 500
    }
    
    Component.onCompleted: {
        var toolTipData = new Object;
        toolTipData["image"] = "preferences-system-time"; 
        toolTipData["mainText"] ="Current Time"
        toolTipData["subText"] = Qt.formatDate( dataSource.data["Local"]["Date"],"dddd, MMM d yyyy" )+"\n"+Qt.formatTime( dataSource.data["Local"]["Time"],"hh:mm:ss AP" )
        plasmoid.popupIconToolTip = toolTipData;
        plasmoid.aspectRatioMode = ConstrainedSquare;
    }
    
    Calendar {
        id:calendar
        anchors.top:parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true;
    }
    
    property Component compactRepresentation: CompactRepresentation {
    }

    Connections {
        target: plasmoid
        onFormFactorChanged: {
            main.formFactor = plasmoid.formFactor
            if(main.formFactor==Planar || main.formFactor == MediaCenter ) {
                minimumWidth=main.width/3.5
                minimumHeight=main.height/3.5
            }
        }
    }
}
