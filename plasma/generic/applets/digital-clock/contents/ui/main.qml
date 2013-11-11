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
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: main
    property int minimumWidth: units.gridUnit * 80
    property int minimumHeight: units.gridUnit * 40
    property int formFactor: plasmoid.formFactor

    PlasmaCore.DataSource {
        id: dataSource
        engine: "time"
        connectedSources: ["Local"]
        interval: 300000
    }

    Component.onCompleted: {
        var toolTipData = new Object;
        toolTipData["image"] = "preferences-system-time"; 
        toolTipData["mainText"] ="Current Time"
        toolTipData["subText"] = Qt.formatDate( dataSource.data["Local"]["Date"],"dddd dd MMM yyyy" )+"\n"+Qt.formatTime( dataSource.data["Local"]["Time"], plasmoid.configuration.timeFormat)
        plasmoid.popupIconToolTip = toolTipData;
        plasmoid.aspectRatioMode = ConstrainedSquare;
    }

    CalendarPopup {
        id: calendar
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
