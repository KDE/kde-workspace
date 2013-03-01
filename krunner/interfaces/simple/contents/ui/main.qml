/*
 *   Copyright 2013 Aaron Seigo aseigo@kde.org
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

Item {
    id: root

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaCore.RunnerModel {
        id: runnerModel
        runners: krunner.runners
        query: queryText.text
    }

    PlasmaComponents.TextField {
        id: queryText

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
    }

    ListView {
        anchors {
            top: queryText.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            topMargin: 3
        }

        clip: true
        spacing: 3
        model: runnerModel
        delegate: Item {
            height: theme.iconSizes.dialog
            width: parent.width
            QIconItem {
                id: icon
                anchors {
                    top: parent.top
                    left: parent.left
                    bottom: parent.bottom
                }
                width: theme.iconSizes.dialog
                icon: display
            }

            Text {
                anchors {
                    top: parent.top
                    left: icon.right
                    right: parent.right
                    bottom: parent.bottom
                }

                text: label
            }
        }
    }
}
