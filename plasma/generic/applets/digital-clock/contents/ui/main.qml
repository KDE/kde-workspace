/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
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
import org.kde.plasma.extras 2.0 as PlasmaExtras
//import org.kde.plasma.calendar 2.0

Item {
    id: main
//     property int minimumWidth: units.gridUnit * 90
//     property int minimumHeight: units.gridUnit * 30
    property int minimumWidth: 360
    property int minimumHeight: 280
    property int formFactor: plasmoid.formFactor

    property alias calendarLoader: calendarLoader

    property Component compactRepresentation: DigitalClock { }

    PlasmaCore.DataSource {
        id: dataSource
        engine: "time"
        connectedSources: ["Local"]
        interval: 300000
    }


    Loader {
        id: calendarLoader
        anchors.fill: parent
        opacity: (calendarLoader.status == Loader.Ready) ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
    }

    PlasmaExtras.Heading {
        id: loadingItem
        ////text: Qt.formatDate(new Date());
//         text: "Loading Calendar ..."
        text: Qt.formatDate( dataSource.data["Local"]["Date"],"dddd, MMM d" )
        anchors {
            left: parent.left
            top: parent.top
            right: parent.right
        }
        opacity: (calendarLoader.status == Loader.Ready) ? 0 : 0.7
        Behavior on opacity { NumberAnimation { duration: 200 } }
    }

    PlasmaComponents.Label {
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        text: Qt.formatDateTime(new Date(), "yyyy")
        font.pixelSize: parent.height / 6
        opacity: loadingItem.opacity
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

    Component.onCompleted: {
        var toolTipData = new Object;
        toolTipData["image"] = "preferences-system-time";
        toolTipData["mainText"] ="Current Time"
        toolTipData["subText"] = Qt.formatDate( dataSource.data["Local"]["Date"],"dddd dd MMM yyyy" )+"\n"+Qt.formatTime( dataSource.data["Local"]["Time"], "HH:MM")
        plasmoid.popupIconToolTip = toolTipData;
        plasmoid.aspectRatioMode = ConstrainedSquare;
    }
}
