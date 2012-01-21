/***************************************************************************
 *   Copyright (C) 2012 by Ingomar Wesp <ingomar@wesp.name>                *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************/
import QtQuick 1.1
import org.kde.qtextracomponents 0.1 as QtExtraComponents

Item {
    property int minIconSize: 16
    property int maxIconSize: 64

    property int maxSectionCount: 0

    property alias model: grid.model
    property alias count: grid.count

    GridView {
        id: grid

        cellWidth: 32
        cellHeight: 32

        anchors.fill: parent

        delegate: QtExtraComponents.QIconItem {
            anchors.margins: 2
            width: grid.cellWidth - 2*anchors.margins
            height: grid.cellHeight - 2*anchors.margins

            icon: QIcon(iconSource)
        }
    }
}
