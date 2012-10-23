/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>

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
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.kickoff 0.1 as Kickoff
Item {
    id: searchViewContainer
    objectName: "SearchView"

    PlasmaComponents.ContextMenu {
        id: contextMenu

        property string title
        property variant icon
        property string url
        property bool favorite: favoritesModel.isFavorite(contextMenu.url)

        function openAt(title, icon, url, x, y) {
            contextMenu.title = title
            contextMenu.icon = icon
            contextMenu.url = url
            open(x, y)
        }

        /*
        * context menu items
        */
        PlasmaComponents.MenuItem {
            id: titleMenuItem
            text: contextMenu.title
            icon: contextMenu.icon
            font.bold: true
            checkable: false
        }
        PlasmaComponents.MenuItem {
            id: titleSeparator
            separator: true
        }
        PlasmaComponents.MenuItem {
            id: addToFavorites
            text: contextMenu.favorite ? i18n("Remove From Favorites") : i18n("Add To Favorites")
            icon: contextMenu.favorite ? QIcon("list-remove") : QIcon("bookmark-new")
            onClicked: {
                if (contextMenu.favorite) {
                    favoritesModel.remove(contextMenu.url);
                } else {
                    favoritesModel.add(contextMenu.url);
                }
            }
        }
        PlasmaComponents.MenuItem {
            id: uninstallApp
            text: i18n("Uninstall")
            enabled: packagekitSource.data["Status"] && packagekitSource.data["Status"]["available"]
            onClicked: {
                var service = packagekitSource.serviceForSource("Status")
                var operation = service.operationDescription("uninstallApplication")
                operation.Url = contextMenu.url;
                var job = service.startOperationCall(operation)
            }
        }
    }

    function decrementCurrentIndex() {
        searchView.decrementCurrentIndex();
    }
    function incrementCurrentIndex() {
        searchView.incrementCurrentIndex();
    }
    function activateCurrentIndex() {
        searchView.currentItem.activate();
    }
    anchors.fill: parent
    PlasmaExtras.ScrollArea {
        anchors.fill: parent
        ListView {
            id: searchView

            anchors.fill: parent
            model: Kickoff.KRunnerModel{}
            delegate: KickoffItem {}
            highlight: PlasmaComponents.Highlight {}
            Connections {
                target: searchBar
                onQueryChanged: {
                    searchView.model.setQuery(searchBar.query)
                }
            }
        }
    }
}
