/*
    Copyright 2013 Heena Mahour <heena393@gmail.com>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This plasmoid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this plasmoid. If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 1.1
import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1 as QtExtraComponents
import org.kde.plasma.components 0.1 as Components
import org.kde.locale 0.1


Item {

    id: mainWindow
    property int minimumHeight: 150
    property int minimumWidth: 250
    property string textColor
    property string shadowColor
    property string textFont
    property bool bold
    property bool italic
    property bool shadow
   
    
    Component.onCompleted: {
        plasmoid.setBackgroundHints( 0 )
        plasmoid.addEventListener( 'ConfigChanged', configChanged );
        textColor = plasmoid.readConfig( "textColor" )
        shadowColor =plasmoid.readConfig( "shadowColor" )
        textFont = plasmoid.readConfig( "textFont" )
        bold = plasmoid.readConfig("bold")
        italic =plasmoid.readConfig("italic")
        shadow = plasmoid.readConfig("shadow")
    }
    
    function configChanged()
    {
        textColor = plasmoid.readConfig( "textColor" )
        shadowColor =plasmoid.readConfig( "shadowColor" )
        textFont = plasmoid.readConfig( "textFont" )
        bold = plasmoid.readConfig("bold")
        italic =plasmoid.readConfig("italic")
        shadow = plasmoid.readConfig("shadow")
    }
    
    function setTimeFormat()
    {
        timeFormat = plasmoid.readConfig( "timeFormat" )
        if( timeFormat == 12 ){
            timeString = (Qt.formatTime( dataSource.data["Local"]["Time"],"h:mmap" )).toString().slice(0,-2)
        } else {
            timeString = (Qt.formatTime( dataSource.data["Local"]["Time"],"hh:mm" ))
        }
    }
  
     Components.Label  {
        id: time
        font.family:textFont
        font.bold: bold?true:false
        style:  shadow?Text.Raised:Text.Normal
        styleColor:shadowcolor? shadowColor: "transparent"
        font.italic:italic?true:false
        color: textColor
        font.pointSize: 30
        text : (Qt.formatTime( dataSource.data["Local"]["Time"],"hh:mmap" )).toString().slice(0,-2)
        anchors {
            top: parent.top;
            left: parent.left;
        }
    }
    
     Components.Label  {
        id: ampm
        font.family:textFont
        font.bold: bold?true:false
         font.italic:italic?true:false
           style:  shadow?Text.Outline:Text.Normal
        styleColor:shadow? shadowColor: "transparent"
       // opacity: 0.5
        color: textColor
        font.pointSize: 30
        text : Qt.formatTime( dataSource.data["Local"]["Time"],"ap" )
        anchors {
            top: parent.top;
            left: time.right;
        }
    }


     Components.Label  {
        id: date
        font.family:textFont
        font.bold: bold?true:false
         font.italic:italic?true:false
           style:  shadow?Text.Outline:Text.Normal
        styleColor:shadowcolor? shadowColor: "transparent"
        color: textColor
       // opacity: 0.5
        font.pointSize: 30
        text : Qt.formatDate( dataSource.data["Local"]["Date"],"dddd, MMM dd" )
        anchors {
            top: time.bottom;
            left: parent.left;
        }
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
}
    
    
