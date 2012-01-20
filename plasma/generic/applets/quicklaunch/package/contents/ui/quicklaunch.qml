/***************************************************************************
 *   Copyright (C) 2010 - 2011 by Ingomar Wesp <ingomar@wesp.name>         *
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
import org.kde.plasma.components 0.1 as PlasmaComponents
// import org.kde.qtextracomponents 0.1 as QtExtraComponents

import org.kde.plasma.quicklaunch 1.0 as Quicklaunch

Item {
    property int minimumWidth: 100
    property int minimumHeight: 100

    Component.onCompleted:
    {
        plasmoid.addEventListener("ConfigChanged", onConfigChanged);
    }

    function onConfigChanged() {
        var sectionCount = plasmoid.readConfig("sectionCount");
        var launcherNamesVisible = plasmoid.readConfig("launcherNamesVisible");

        popupTrigger.enabled = plasmoid.readConfig("popupEnabled");

        var launchers = new String(plasmoid.readConfig("launchers")).split(",");
        var launchersOnPopup =
            new String(plasmoid.readConfig("launchersOnPopup")).split(",");

        for (i in launchers) {
            launcherListModel.addLauncher(i, launchers[i]);
        }
    }

    PlasmaCore.Svg {
       id: arrowsSvg
       imagePath: "widgets/arrows"
    }

    PlasmaCore.SvgItem {
        id: popupTrigger

        property bool enabled: false;

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        svg: arrowsSvg
        elementId: "up-arrow"

        width: enabled ? 16 : 0;
        height: enabled ? 16 : 0;
        visible: enabled;
    }

    ListView {
        id: launcherList

        anchors {
            top: parent.top
            left: parent.left
            right: popupTrigger.left
            bottom: parent.bottom
        }

        model: Quicklaunch.LauncherListModel {
            id: launcherListModel
        }

        delegate: PlasmaComponents.Button {
            text: display
            iconSource: icon
        }
    }
}
