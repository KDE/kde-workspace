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
import org.kde.qtextracomponents 0.1
import "data.js" as Data
import org.kde.dirmodel 0.1
Item {
    id:root
    property int minimumWidth:formFactor == Horizontal ? height : 1
    property int minimumHeight:formFactor == Vertical ? width  : 1
    property int formFactor: plasmoid.formFactor
    DirModel {
        id:dirModel
        url: "trash:/"
    }
    Connections {
        target: plasmoid
        onFormFactorChanged: {
            root.formFactor = plasmoid.formFactor
        }
    }
    function action_open() {
        plasmoid.openUrl("trash:/");
    }
    function action_empty() {
        plasmoid.runCommand("ktrash", ["--empty"]);
    }
    Component.onCompleted: { 
        plasmoid.setBackgroundHints( 0 )
        plasmoid.action_open = function() {
            plasmoid.openUrl("trash:/");
        }
        plasmoid.setAction("open", i18n("Open"),"document-open");
        plasmoid.action_empty=function() {
            plasmoid.runCommand("ktrash", ["--empty"]);
        }
        plasmoid.setAction("empty",i18n("Empty"),"trash-empty");
        plasmoid.popupIcon = QIcon("user-trash");
        plasmoid.aspectRatioMode = IgnoreAspectRatio;
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onReleased: plasmoid.openUrl("trash:/");
        Row {
            id:row
            spacing:0
            PlasmaCore.IconItem {
                id:icon
                width:root.width
                height:width
                source: (dirModel.count > 0) ? "user-trash-full" : "user-trash"
            }
            Components.Label {
                id:text
                color:theme.textColor
                font.bold:false
                font.pointSize:root.width/10
                text:"Trash \n"+dirModel.count
                anchors {
                    horizontalCenter:icon.horizontalCenter
                    top:icon.bottom
                }
            }
        }
    }
}