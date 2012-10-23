/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012 Marco Martin <mart@kde.org>

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
import org.kde.plasma.kickoff 0.1 as Kickoff
import org.kde.plasma.components 0.1 as PlasmaComponents


BaseView {
    objectName: "RecentlyUsedView"

    ContextMenu {
        id: contextMenu

        PlasmaComponents.MenuItem {
            id: addToFavorites
            text: contextMenu.favorite ? i18n("Remove From Favorites") : i18n("Add To Favorites")
            icon: contextMenu.favorite ? QIcon("list-remove") : QIcon("bookmark-new")
            onClicked: {
                if (contextMenu.favorite) {
                    favoritesModel.remove(contextMenu.model.url);
                } else {
                    favoritesModel.add(contextMenu.model.url);
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
                operation.Url = contextMenu.model.url;
                var job = service.startOperationCall(operation)
            }
        }
        PlasmaComponents.MenuItem {
            id: actionsSeparator
            separator: true
        }
        PlasmaComponents.MenuItem {
            id: clearRecentApplications
            text: i18n("Clear Recent Applications")
            icon: QIcon("edit-clear-history")
            onClicked: recentlyUsedModel.clearRecentApplications();
        }
        PlasmaComponents.MenuItem {
            id: clearRecentDocuments
            text: i18n("Clear Recent Documents")
            icon: QIcon("edit-clear-history")
            onClicked: recentlyUsedModel.clearRecentDocuments();
        }
    }

    model: Kickoff.RecentlyUsedModel {
        id: recentlyUsedModel
    }
}
