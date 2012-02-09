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
    property int minimumWidth: iconGrid.minimumWidth + popupLoader.width
    property int minimumHeight: iconGrid.minimumHeight

    clip: true

    Component.onCompleted:
    {
        plasmoid.addEventListener("configChanged", onConfigChanged);
        // plasmoid.addEventListener("locationChanged", onLocationChanged);

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

        if (autoSectionCountEnabled) {
            sectionCount = 0;
        }

        iconGrid.maxSectionCount = sectionCount;

        if (plasmoid.readConfig("popupEnabled") == true) {
            popupLoader.source = "popup.qml";
        } else {
            popupLoader.sourceComponent = undefined;
        }

        // XXX: Workaround for bug 267809. This should be declared as a
        // StringList rather than a String in config/main.xml
        var launchers = new String(plasmoid.readConfig("launchers")).split(",");
        var launchersOnPopup = new String(plasmoid.readConfig("launchersOnPopup")).split(",");

        // Repopulate launcher list.
        iconGrid.model.clear();
        for (i in launchers) {
            if (launchers[i].length > 0) {
                iconGrid.model.addLauncher(i, launchers[i]);
            }
        }

        if (iconGrid.count == 0) {
            iconGrid.model.restoreDefaultLaunchers();
        }

        // Repopulate popup launcher list.
        if (popupLoader.status == Loader.Ready) {
            popupLoader.item.model.clear();
            for (i in launchersOnPopup) {
                if (launchersOnPopup[i].length > 0) {
                    popupLoader.item.model.addLauncher(i, launchersOnPopup[i]);
                }
            }
        }
    }

    IconGrid {
        id: iconGrid
        anchors {
            top: parent.top
            left: parent.left
            right: popupLoader.left
            bottom: parent.bottom
        }

        model: Quicklaunch.LauncherListModel {}
    }

    Loader {
        id: popupLoader

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
    }
}
