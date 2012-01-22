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

import org.kde.plasma.quicklaunch 1.0 as Quicklaunch

Item {
    property int minimumWidth: 20 + popupTrigger.height
    property int minimumHeight: 20

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
        var autoSectionCountEnabled = plasmoid.readConfig("autoSectionCountEnabled");
        var sectionCount = plasmoid.readConfig("sectionCount");
        var launcherNamesVisible = plasmoid.readConfig("launcherNamesVisible");

        if (sectionCount < 1) {
            autoSectionCountEnabled = true;
        }

        popupTrigger.visible = plasmoid.readConfig("popupEnabled");
        if (popupTrigger.visible) {
            updatePopupTrigger();
        }

        // XXX: Workaround for bug 267809. This should be declared as a
        // StringList rather than a String in config/main.xml
        var launchers = new String(plasmoid.readConfig("launchers")).split(",");
        var launchersOnPopup = new String(plasmoid.readConfig("launchersOnPopup")).split(",");

        // Repopulate launcher list.
        launcherList.model.clear();
        for (i in launchers) {
            if (launchers[i].length > 0) {
                launcherList.model.addLauncher(i, launchers[i]);
                print("Adding launcher: "+launchers[i]);
            }
        }

        // Repopulate popup launcher list.
        popup.model.clear();
        for (i in launchersOnPopup) {
            if (launchersOnPopup[i].length > 0) {
                popup.model.addLauncher(i, launchersOnPopup[i]);
                print("Adding launcher: "+launchersOnPopup[i]);
            }
        }

        print("Launchers in model: "+launcherList.count);
    }

    function updatePopupTrigger() {
        switch(plasmoid.location) {
            case TopEdge:
                popupTrigger.elementId = popup.visible ? "up-arrow" : "down-arrow";
                break;
            case LeftEdge:
                popupTrigger.elementId = popup.visible ? "left-arrow" : "right-arrow";
                break;
            case RightEdge:
                popupTrigger.elementId = popup.visible ? "right-arrow" : "left-arrow";
                break;
            default:
                popupTrigger.elementId = popup.visible ? "down-arrow" : "up-arrow";
        }
    }

    PlasmaCore.Svg {
       id: arrowsSvg
       imagePath: "widgets/arrows"
    }

    PlasmaCore.SvgItem {
        id: popupTrigger

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        svg: arrowsSvg
        elementId: "up-arrow"

        visible: false;
        width: visible ? 16 : 0;
        height: visible ? 16 : 0;

        MouseArea {
            anchors.fill: parent
            onClicked: {
                var popupPosition = popup.popupPosition(popupTrigger);
                popup.dialogX = popupPosition.x;
                popup.dialogY = popupPosition.y;
                popup.visible = !popup.visible;
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

    Popup {
        id: popup
        model: Quicklaunch.LauncherListModel {}
    }
}
