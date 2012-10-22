/***************************************************************************
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtras

PlasmaComponents.ListItem {
    id: toolBox
    property QtObject action
    property alias iconSource: iconItem.icon
    property alias text: label.text
    signal triggered
    enabled: true

    height: main.iconSize*1.8

    QtExtras.QIconItem {
        id: iconItem
        height: iconSize
        width: iconSize
        anchors { left: parent.left; verticalCenter: parent.verticalCenter; }
    }
    PlasmaComponents.Label {
        id: label
        elide: Text.ElideMiddle
        anchors { left: iconItem.right; right: parent.right; leftMargin: 6; verticalCenter: parent.verticalCenter; }
    }

    onClicked: {
        if (action) {
            action.trigger()
        }
        triggered();
    }
}
