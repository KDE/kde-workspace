/*
 * Copyright 2013 Jörg Ehrichs <joerg.ehrichs@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

import "plasmapackage:/code/logic.js" as Logic

/**
 * Base item that contains the list of all available Nepomuk indexer
 *
 * Loads the Plasm::DataEngine and shows detailed information for the services
 * - FileWatch
 * - FileIndexer
 * - Akonadi Nepomuk Feeder
 * - WebMiner
 */
Item {
    id: nepomukcontroller
    property int minimumWidth: 450
    property int minimumHeight: mainColumn.height + mainButtonRow.height + 10
    property int maximumHeight: mainColumn.height + mainButtonRow.height + 10
    property int implicitHeight: mainColumn.height + mainButtonRow.height + 10

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaCore.DataSource {
        id: nepomukSource
        engine: "nepomuk-serviceengine"
        connectedSources: sources
        interval: 0

        onDataChanged: {
            Logic.updateTooltip();

            // only change active/passive if applet is not opened
            if (data["Nepomuk"]["isActive"] && plasmoid.status != 3) {
                plasmoid.status = "ActiveStatus"
            }
            else if ( !data["Nepomuk"]["isActive"] && plasmoid.status != 3) {
                plasmoid.status = "PassiveStatus"
            }
        }
    }

    Component.onCompleted: {
        plasmoid.addEventListener ('ConfigChanged', Logic.configChanged);

        Logic.updateTooltip()
        Logic.configChanged()
    }

    PlasmaCore.Svg {
        id: lineSvg
        imagePath: "widgets/line"
    }


    Column {
        id: mainColumn
        width: parent.width
        height: childrenRect.height
        spacing: 5

        ServiceItem {
            id: fileWatcher

            width: parent.width
            visible: plasmoid.readConfig("showFileWatch");

            name: nepomukSource.data["FileWatch"]["i18nName"]
            uid: "FileWatch"
            buttons:     nepomukSource.data["FileWatch"]["canBeSuspended"]
            statusMsg:   nepomukSource.data["FileWatch"]["statusMessage"]
            status:      nepomukSource.data["FileWatch"]["status"]
            isAvailable: nepomukSource.data["FileWatch"]["isAvailable"]
            isSuspended: nepomukSource.data["FileWatch"]["isSuspended"]
            isActive:    nepomukSource.data["FileWatch"]["isActive"]
            percent:     nepomukSource.data["FileWatch"]["percent"]
        }

        ServiceItem {
            id: fileIndexer

            width: parent.width
            visible: plasmoid.readConfig("showFileIndexer");

            name: nepomukSource.data["FileIndexer"]["i18nName"]
            uid: "FileIndexer"
            buttons:     nepomukSource.data["FileIndexer"]["canBeSuspended"]
            statusMsg:   nepomukSource.data["FileIndexer"]["statusMessage"]
            status:      nepomukSource.data["FileIndexer"]["status"]
            isAvailable: nepomukSource.data["FileIndexer"]["isAvailable"]
            isSuspended: nepomukSource.data["FileIndexer"]["isSuspended"]
            isActive:    nepomukSource.data["FileIndexer"]["isActive"]
            percent:     nepomukSource.data["FileIndexer"]["percent"]
        }

        ServiceItem {
            id: akonadiFeeder

            width: parent.width
            visible: plasmoid.readConfig("showPIMIndexer");

            name: nepomukSource.data["PIM"]["i18nName"]
            uid: "PIM"
            buttons:     nepomukSource.data["PIM"]["canBeSuspended"]
            statusMsg:   nepomukSource.data["PIM"]["statusMessage"]
            status:      nepomukSource.data["PIM"]["status"]
            isAvailable: nepomukSource.data["PIM"]["isAvailable"]
            isSuspended: nepomukSource.data["PIM"]["isSuspended"]
            isActive:    nepomukSource.data["PIM"]["isActive"]
            percent:     nepomukSource.data["PIM"]["percent"]
        }

        ServiceItem {
            id: webMiner

            width: parent.width
            visible: plasmoid.readConfig("showWebMiner");

            name: nepomukSource.data["WebMiner"]["i18nName"]
            uid: "WebMiner"
            buttons:     nepomukSource.data["WebMiner"]["canBeSuspended"]
            statusMsg:   nepomukSource.data["WebMiner"]["statusMessage"]
            status:      nepomukSource.data["WebMiner"]["status"]
            isAvailable: nepomukSource.data["WebMiner"]["isAvailable"]
            isSuspended: nepomukSource.data["WebMiner"]["isSuspended"]
            isActive:    nepomukSource.data["WebMiner"]["isActive"]
            percent:     nepomukSource.data["WebMiner"]["percent"]
        }
    }

    PlasmaComponents.ButtonRow {
        id: mainButtonRow
        exclusive: false

        anchors {
            right: parent.right
            bottom: parent.bottom
            bottomMargin: 5
        }

        PlasmaComponents.Button {
            id: enableAll
            iconSource: "media-playback-start"
            text: i18n("Resume All")

            onClicked: {
                service = nepomukSource.serviceForSource("Nepomuk");
                operation = service.operationDescription("resumeAll");
                operation.predicate = "resumeAll";
                service.startOperationCall(operation);
            }
        }

        PlasmaComponents.Button {
            id: stopAll
            iconSource: "media-playback-stop"
            text: i18n("Suspend All")

            onClicked: {
                service = nepomukSource.serviceForSource("Nepomuk");
                operation = service.operationDescription("suspendAll");
                operation.predicate = "suspendAll";
                service.startOperationCall(operation);
            }
        }
    }
}
