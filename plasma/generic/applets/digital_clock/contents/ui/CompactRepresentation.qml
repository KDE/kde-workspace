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
import org.kde.plasma.components 2.0 as Components
import org.kde.locale 2.0

Item {
    id: main
    property int minimumWidth
    property int minimumHeight
    property int formFactor: plasmoid.formFactor
    property bool constrained: formFactor == PlasmaCore.Types.Vertical || formFactor == PlasmaCore.Types.Horizontal

    Locale {
        id: locale
    }

    PlasmaCore.DataSource {
        id: dataSource
        engine: "time"
        connectedSources: ["Local"]
        interval: 300000
    }

    Components.Label  {
        id: time
        font.bold: plasmoid.configuration.boldText
        font.italic: plasmoid.configuration.italicText
        font.pixelSize: Math.min(main.width/6, main.height)
        width: Math.max(paintedWidth,time.paintedWidth)
        //FIXME: Fix enums in locale bindings.
        text: {
            if( plasmoid.configuration.showSeconds )
                return locale.formatLocaleTime( dataSource.data["Local"]["Time"], Locale.TimeWithSeconds );
            else
                return locale.formatLocaleTime( dataSource.data["Local"]["Time"], Locale.TimeWithoutSeconds );
        }
        horizontalAlignment: main.AlignHCenter
        anchors {
            centerIn: parent
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: plasmoid.expanded = !plasmoid.expanded;

            PlasmaCore.ToolTip {
                id: tooltip
                target: time
                mainText: "Current Time"
                subText: Qt.formatDate( dataSource.data["Local"]["Date"],"dddd, MMM d yyyy" ) + "\n" + locale.formatLocaleTime(dataSource.data["Local"]["Time"], Locale.TimeWithoutSeconds)
                image: "preferences-system-time"
            }
        }
    }
}
