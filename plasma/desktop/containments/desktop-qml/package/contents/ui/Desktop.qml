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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.plasma.components 0.1 as PlasmaComponents

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

Item {
    id: main
    width: 540
    height: 540

    Rectangle {
        color: "grey"
        opacity: .2
        anchors.fill: parent
        anchors.margins: 24
    }

    Grid {
        property int cellWidth: 24
        property int cellHeight: 24
        model: gridModel
        anchors.fill: parent
        delegate: Rectangle {
            width: cellWidth
            height: cellHeight
            color: theme.backgroundColor
            opacity: 0.3

        }

        ListModel {
            id: gridModel
            Component.onCompleted: {
                var cells  = (main.width / cellWidth) * (main.height / cellHeight);
                print(" Got " + cells + " Cells.");
                for (i = 0; i < cells; i++) {
                    gridModel.append({"numbor": id, "name":"Jackfruit"});
                }

            }
        }
    }

    Item {
        id: resultsFlow
        anchors.fill: parent
    }

    PlasmaComponents.Button {
        id: addAppletButton
        text: "Add Applet"
        iconSource: "list-add"
        anchors { top: parent.top; right: parent.right; margins: 12; }
        onClicked: {
            var ap = "digital-clock";
            print("Adding applet..." + ap);
        }
    }
    Component.onCompleted: {
        print("\n\n\n");
        //do it here since theme is not accessible in LayoutManager
        //TODO: icon size from the configuration
        //TODO: remove hardcoded sizes, use framesvg boders
        //updateGridSize()

        plasmoid.containmentType = "CustomContainment"
        plasmoid.appletAdded.connect(addApplet)
        LayoutManager.restore()
        for (var i = 0; i < plasmoid.applets.length; ++i) {
            var applet = plasmoid.applets[i]
            addApplet(applet, 0)
        }
    }

    function addApplet(applet, pos)
    {
        print("ADD APPLET! " + applet.id + " " + pos);
        var component = Qt.createComponent("PlasmoidGroup.qml")
        var plasmoidGroup = component.createObject(resultsFlow)
        plasmoidGroup.width = LayoutManager.cellSize.width*2
        plasmoidGroup.height = LayoutManager.cellSize.height*2
        plasmoidGroup.applet = applet
        plasmoidGroup.category = "Applet-"+applet.id
        LayoutManager.itemGroups[plasmoidGroup.category] = plasmoidGroup
    }
}
