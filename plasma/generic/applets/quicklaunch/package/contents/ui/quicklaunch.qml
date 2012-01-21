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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtraComponents

import org.kde.plasma.quicklaunch 1.0 as Quicklaunch

Item {
    property int popupTriggerSize: 16

    property int minimumWidth: 20 + popupTrigger.height
    property int minimumHeight: 20

    property bool popupShown: false;

    Component.onCompleted:
    {
        plasmoid.addEventListener("ConfigChanged", onConfigChanged);
        plasmoid.addEventListener("LocationChanged", updatePopupTrigger);

        plasmoid.setAction("addLauncher", i18n("Add Launcher..."), "list-add");
        plasmoid.setAction("editLauncher", i18n("Edit Launcher..."), "document-edit");
        plasmoid.setAction("removeLauncher", i18n("Remove Launcher"), "list-remove");
        plasmoid.setActionSeparator("separator");

        // TODO: Implement
        plasmoid.action_addLauncher = function() {}
        plasmoid.action_editLauncher = function() {}
        plasmoid.action_removeLauncher = function() {}
    }

    function onConfigChanged() {
        var sectionCount = plasmoid.readConfig("sectionCount");
        var launcherNamesVisible = plasmoid.readConfig("launcherNamesVisible");

        popupTrigger.enabled = plasmoid.readConfig("popupEnabled");
        if (popupTrigger.enabled) {
            updatePopupTrigger();
        }

        var launchers = new String(plasmoid.readConfig("launchers")).split(",");
        var launchersOnPopup =
            new String(plasmoid.readConfig("launchersOnPopup")).split(",");

        // Repopulate launcher list.
        launcherList.model.clear();
        for (i in launchers) {
            launcherList.model.addLauncher(i, launchers[i]);
            print("Adding launcher: "+launchers[i]);
        }

        print("Launchers in model: "+launcherList.count);
    }

    function updatePopupTrigger() {
        switch(plasmoid.location) {
            case TopEdge:
                popupTrigger.elementId = popupShown ? "up-arrow" : "down-arrow";
                break;
            case LeftEdge:
                popupTrigger.elementId = popupShown ? "left-arrow" : "right-arrow";
                break;
            case RightEdge:
                popupTrigger.elementId = popupShown ? "right-arrow" : "left-arrow";
                break;
            default:
                popupTrigger.elementId = popupShown ? "down-arrow" : "up-arrow";
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

        width: enabled ? popupTriggerSize : 0;
        height: enabled ? popupTriggerSize : 0;
        visible: enabled;

        MouseArea {
            anchors.fill: parent
            onClicked: {
                popupShown = !popupShown;
                updatePopupTrigger();
            }
        }
    }

    IconGrid {
        id: launcherList
        anchors {
            top: parent.top
            left: parent.left
            right: popupTrigger.left
            bottom: parent.bottom
        }

        model: Quicklaunch.LauncherListModel {}
    }
}
