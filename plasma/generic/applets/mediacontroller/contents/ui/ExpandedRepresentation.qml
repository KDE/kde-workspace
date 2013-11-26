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
    id: expandedRepresentation

    anchors {
        margins: theme.largeSpacing
        //fill: parent
    }

    //Rectangle { color: "orange"; anchors.fill: parent }

    PlasmaExtras.Heading {
        id: song
        level: 1
        anchors {
            left: parent.left
            top: parent.top
            right: parent.right
        }
        elide: Text.ElideRight
        text: root.track
    }

    PlasmaExtras.Heading {
        id: artist
        level: 3
        anchors {
            top: song.bottom
            topMargin: theme.smallSpacing
            left: parent.left
            right: parent.right
        }
        elide: Text.ElideRight
        text: root.artist
    }

    Row {
        id: playerControls
        property int controlsSize: theme.mSize(theme.defaultFont).height * 3

        Rectangle { color: "orange"; anchors.fill: parent }

        anchors {
            top: artist.bottom
            topMargin: theme.largeSpacing
            bottom: parent.bottom
            //horizontalCenter: parent.horizontalCenter
            left: parent.left
        }

        height: root.controlsSize
        //height: 20
        width: (root.controlsSize * 3) + (theme.largeSpacing * 2)
        spacing: theme.largeSpacing

        MediaControl {
            source: "media-skip-backward"
            onTriggered: root.previous();
        }

        MediaControl {
            source: root.state == "playing" ? "media-playback-pause" : "media-playback-start"
            onTriggered: {
                print("Clicked" + source + " " + root.state);
                if (root.state == "playing") {
                    root.pause();
                } else {
                    root.play();
                }
            }
        }

        MediaControl {
            source: "media-skip-forward"
            onTriggered: root.next();
        }
    }
}