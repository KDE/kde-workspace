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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1 as QtExtraComponents

Item {
    id: iconGrid

    property int minCellSize: 24
    property int maxCellSize: 64

    property int maxSectionCount: 0

    // XXX: This should be an enum, really.
    property string mode: "horizontalPanel";

    property int minimumWidth: minCellSize;
    property int minimumHeight: minCellSize;
    property alias model: gridView.model
    property alias count: gridView.count


    GridView {
        id: gridView

        property int cellSize: 16
        cellWidth: cellSize
        cellHeight: cellSize

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        focus: true

        delegate: Item {
            id: delegate
            width: gridView.cellWidth
            height: gridView.cellHeight

            QtExtraComponents.QIconItem {
                anchors.horizontalCenter: parent.horizontalCenter;
                anchors.verticalCenter: parent.verticalCenter;

                width: gridView.cellWidth - 4
                height: gridView.cellHeight - 4

                icon: QIcon(iconSource)
            }

            PlasmaCore.ToolTip {
                target: delegate
                mainText: display
                subText: description
                image: iconSource
            }
        }
    }

    // TODO: Do this onAdd / onRemove as well.
    onMaxSectionCountChanged: updateGridProperties();

    onWidthChanged: {
        if (mode == "verticalPanel") {
            updateGridProperties();
        }
    }

    onHeightChanged: {
        if (mode == "horizontalPanel") {
            updateGridProperties();
        }
    }

    function updateGridProperties() {
        if (mode == "horizontalPanel" || mode == "verticalPanel") {

            var horizontalPanel = (mode == "horizontalPanel");

            var panelThickness =
                horizontalPanel ? iconGrid.height : iconGrid.width;
            var sectionCount = Math.floor(panelThickness / minCellSize);

            // Ensure sectionCount is bounded to [1;min(count,maxSectionCount)].
            if (sectionCount < 1) {
                sectionCount = 1;
            }
            else {
                if (maxSectionCount != 0 && maxSectionCount < sectionCount) {
                    sectionCount = maxSectionCount;
                }
                if (count != 0 && count < sectionCount) {
                    sectionCount = count;
                }
            }

            // Ensure sectionCount is not bigger than it needs to be.
            var maxItemsPerSection = Math.ceil(count / sectionCount);

            while (sectionCount > 1)  {
                var maxItemsPerBiggerSection = Math.ceil(count / (sectionCount-1));

                if (maxItemsPerBiggerSection == maxItemsPerSection) {
                    sectionCount--;
                    maxItemsPerSection = maxItemsPerSection;
                }
                else {
                    break;
                }
            }

            var newCellSize = Math.floor(panelThickness / sectionCount);
            if (newCellSize < minCellSize) {
                newCellSize = minCellSize;
            }
            else if (newCellSize > maxCellSize) {
                newCellSize = maxCellSize;
            }
            gridView.cellSize = newCellSize;

            if (horizontalPanel) {
                gridView.width = maxItemsPerSection * newCellSize;
                gridView.height = sectionCount * newCellSize;

                iconGrid.minimumWidth = maxItemsPerSection * newCellSize;
                iconGrid.minimumHeight = minCellSize;
            } else {
                gridView.height = maxItemsPerSection * newCellSize;
                gridView.width = sectionCount * newCellSize;

                iconGrid.minimumHeight = gridView.height;
                iconGrid.minimumWidth = minCellSize;
            }
        }
    }
}
