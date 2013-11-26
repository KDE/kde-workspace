/***************************************************************************
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root

    property string track: "Rockin' in a free world"
    property string artist: "Pearl Jam & Neil Young"
    property string playerIcon: ""


    property int minimumWidth: minimumHeight * 1.333
    property int minimumHeight: theme.mSize(theme.defaultFont).height * 8
    property int implicitWidth: minimumWidth * 1.5
    property int implicitHeight: minimumHeight * 1.5

    property int baseSize: theme.mSize(theme.defaultFont).height
    property int controlSize: baseSize * 3

    property alias expandedLoader: expandedLoader

    property Component compactRepresentation: CompactRepresentation {
    }

    state: "off"
//     Rectangle { color: "blue"; anchors.fill: parent }

    PlasmaCore.DataSource {
        id: mpris2Source
        engine: "mpris2"
        connectedSources: sources
        interval: 0
        property string last
        onSourceAdded: {
            print("XXX source added: " + source);
            last = source;
            updateData();
        }

        onSourceRemoved: {
            print("XXX source removed: " + sourceName);
        }

        onDataChanged: {
            updateData();
            //print("XXX data changed SOURCE: " + source);
        }

        function updateData() {
            print("XXX Showing data: " + last);
            var d = data[last];

            if (d == undefined) {
                return;
            }

            var _state = d["PlaybackStatus"];
            if (_state == "Paused") {
                root.state = "paused";
            } else if (_state == "Playing") {
                root.state = "playing";
            } else {
                root.state = "off";
            }

            var metadata = d["Metadata"]

            var track = metadata["xesam:title"];
            var artist = metadata["xesam:artist"];

            root.track = track;
            root.artist = artist;

            // other metadata
            var k;
            for (k in metadata) {
                print(" -- " + k + " " + metadata[k]);
            }
        }

    }

    function play() {
        print("PLay!");
        serviceOp(mpris2Source.last, "Play");

    }

    function pause() {
        print("Pause!");
        serviceOp(mpris2Source.last, "Pause");
    }

    function previous() {
        print("Previous!");
        serviceOp(mpris2Source.last, "Previous");
    }

    function next() {
        print("Next!");
        serviceOp(mpris2Source.last, "Next");
    }

    function serviceOp(src, op) {
        //print(" serviceOp: " + src + " Op: " + op);
        var service = mpris2Source.serviceForSource(src);
        var operation = service.operationDescription(op);
        return service.startOperationCall(operation);
    }

    Loader {
        id: expandedLoader
        anchors.fill: parent
    }

    Timer {
        interval: 500
        running: true
        onTriggered: {
            if (plasmoid.expanded) {
                if (expandedLoader.source == "") {
                    expandedLoader.setSource("ExpandedRepresentation.qml", { 'anchors.fill': root});
                }
            }
        }
    }

    states: [
        State {
            name: "off"
        },
        State {
            name: "playing"
        },
        State {
            name: "paused"
        }
    ]

    onStateChanged: {
        print(">>> XXXX State is now: " + root.state);
    }
}