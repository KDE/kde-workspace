/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2012  Marco Martin <mart@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.private.kickoff 0.1 as Kickoff
import org.kde.qtextracomponents 2.0


Item {
    id: searchBar

    height: Math.max(searchIcon.height+4, searchField.height+4)

    property alias query: searchField.text

    onFocusChanged: {
        searchField.forceActiveFocus();
    }

    QIconItem {
        id: searchIcon

        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: 4
        }
        height: 32
        width: 32

        icon: "system-search"
    }

    PlasmaComponents.Label {
        id: searchLabel

        anchors {
            left: searchIcon.right
            verticalCenter: parent.verticalCenter
            leftMargin: y
        }

        text: i18n("Search:")
    }

    PlasmaComponents.TextField {
        id: searchField

        anchors {
            left: searchLabel.right
            right: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: y
            rightMargin: y
        }

        placeholderText: i18nc("Search field placeholder text", "Search")
        clearButtonShown: true

        onTextChanged: {
            if (root.state != "Search") {
                root.previousState = root.state;
                root.state = "Search";
            }
            if (text == "") {
                root.state = root.previousState;
                root.forceActiveFocus();
            }
        }
    } // searchField
} // searchBar
