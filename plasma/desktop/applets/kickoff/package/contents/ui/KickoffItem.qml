/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>

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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1
import org.kde.draganddrop 1.0

PlasmaComponents.ListItem {
    id: listItem
    enabled: true
    clip: true
    checked: ListView.isCurrentItem
    height: Math.max(elementIcon.height, titleElement.paintedHeight + subTitleElement.paintedHeight)

    property bool modelChildren: hasModelChildren

    function activate() {
        if (hasModelChildren) {
            listItem.ListView.view.addBreadcrumb(listItem.ListView.view.model.modelIndex(index), display);
            listItem.ListView.view.model.rootIndex = listItem.ListView.view.model.modelIndex(index);
        } else {
            launcher.openUrl(model["url"]);
            plasmoid.hidePopup();
        }
    }
    QIconItem {
        id: elementIcon
        icon: decoration
        width: 32
        height: 32
        anchors {
            left: parent.left
        }
    }
    PlasmaComponents.Label {
        id: titleElement
        text: {
            if (root.state == "APPLICATIONS") {
                if (hasModelChildren) {
                    return display;
                } else {
                    // TODO: games should always show the by name
                    return root.showAppsByName ? subtitle : display;
                }
            } else {
                return display;
            }
        }
        anchors {
            top: parent.top
            left: elementIcon.right
            right: parent.right
            leftMargin: 5
        }
    }
    PlasmaComponents.Label {
        id: subTitleElement
        text: {
            if (root.state == "APPLICATIONS") {
                if (hasModelChildren) {
                    return "";
                } else {
                    return root.showAppsByName ? display : subtitle;
                }
            } else {
                return subtitle;
            }
        }
        visible: listItem.ListView.isCurrentItem
        opacity: 0.6
        font.pointSize: theme.smallestFont.pointSize
        elide: Text.ElideMiddle
        anchors {
            left: elementIcon.right
            right: parent.right
            bottom: parent.bottom
            top: titleElement.bottom
            leftMargin: 5
        }
    }

    PlasmaComponents.ContextMenu {
        id: contextMenu
    }
    /*
     * context menu items
     */
    PlasmaComponents.MenuItem {
        id: titleMenuItem
        text: titleElement.text
        icon: decoration
        font.bold: true
        checkable: false
    }
    PlasmaComponents.MenuItem {
        id: titleSeparator
        separator: true
    }
    PlasmaComponents.MenuItem {
        id: actionsSeparator
        separator: true
    }
    PlasmaComponents.MenuItem {
        id: addToFavorites
        text: "Add To Favorites"
        icon: QIcon("bookmark-new")
        onClicked: {
            listItem.ListView.view.favoritesModel.add(model["url"]);
        }
    }
    PlasmaComponents.MenuItem {
        id: removeFromFavorites
        text: "Remove From Favorites"
        icon: QIcon("list-remove")
        onClicked: {
            listItem.ListView.view.favoritesModel.remove(model["url"]);
        }
    }
    PlasmaComponents.MenuItem {
        id: sortFavoritesAscending
        text: "Sort Alphabetically (A to Z)"
        icon: QIcon("view-sort-ascending")
        onClicked: {
            listItem.ListView.view.favoritesModel.sortFavoritesAscending();
        }
    }
    PlasmaComponents.MenuItem {
        id: sortFavoritesDescending
        text: "Sort Alphabetically (Z to A)"
        icon: QIcon("view-sort-descending")
        onClicked: {
            listItem.ListView.view.favoritesModel.sortFavoritesDescending();
        }
    }
    PlasmaComponents.MenuItem {
        id: clearRecentApplications
        text: "Clear Recent Applications"
        icon: QIcon("edit-clear-history")
        onClicked: {
            listItem.ListView.view.recentlyUsedModel.clearRecentApplications();
        }
    }
    PlasmaComponents.MenuItem {
        id: clearRecentDocuments
        text: "Clear Recent Documents"
        icon: QIcon("edit-clear-history")
        onClicked: {
            listItem.ListView.view.recentlyUsedModel.clearRecentDocuments();
        }
    }
    PlasmaComponents.MenuItem {
        id: uninstallApp
        text: "Uninstall"
        onClicked: {
            print("uninstall " +  model["url"]);
            var service = packagekitSource.serviceForSource("Status")
            var operation = service.operationDescription("uninstallApplication")
            operation.Url = model["url"];
            var job = service.startOperationCall(operation)
        }
    }

    DragArea {
        anchors.fill: parent
        supportedActions: Qt.MoveAction | Qt.LinkAction
        delegateImage: decoration
            mimeData {
                url: model["url"]
                source: parent
                text: index
            }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onEntered: {
                listItem.ListView.view.currentIndex = index;
            }
            onClicked: {
                if (mouse.button == Qt.LeftButton)
                    activate();
                else if (mouse.button == Qt.RightButton) {
                    // don't show a context menu for container
                    if (hasModelChildren)
                        return;

                    contextMenu.addMenuItem(titleMenuItem)
                    contextMenu.addMenuItem(titleSeparator)

                    if (listItem.ListView.view.favoritesModel.isFavorite(model["url"]))
                        contextMenu.addMenuItem(removeFromFavorites)
                    else {
                        if (listItem.ListView.view.model == listItem.ListView.view.recentlyUsedModel ||
                            root.state == "APPLICATIONS" ||
                            root.state == "SEARCH") {
                            contextMenu.addMenuItem(addToFavorites);
                        }
                    }

                    if (packagekitSource.data["Status"]["available"]) {
                        contextMenu.addMenuItem(uninstallApp);
                    }

                    if (root.state == "NORMAL") {
                        contextMenu.addMenuItem(actionsSeparator)
                        if (listItem.ListView.view.model == listItem.ListView.view.favoritesModel) {
                            contextMenu.addMenuItem(sortFavoritesAscending)
                            contextMenu.addMenuItem(sortFavoritesDescending)
                        } else if (listItem.ListView.view.model == listItem.ListView.view.recentlyUsedModel) {
                            contextMenu.addMenuItem(clearRecentApplications);
                            contextMenu.addMenuItem(clearRecentDocuments);
                        }
                    }

                    var mapPos = listItem.mapToItem(listItem.ListView.view.parent.parent.parent, mouse.x, mouse.y);
                    contextMenu.open(mapPos.x,mapPos.y);
                }
            }
        }
    }
    DropArea {
        // We could anchors.fill: parent here, but then, dropping in between items is possible,
        // so we make the drop area a bit larger than the item itself
        anchors {
            verticalCenter: parent.verticalCenter;
            left: parent.left;
            right: parent.right;
        }
        height: parent.height+8
        onDrop: {
            listItem.ListView.view.model.dropMimeData(event.mimeData.text, event.mimeData.urls, index, 0);
        }
        onDragEnter: {
            listItem.checked = true;
        }
        onDragLeave: {
            listItem.checked = false;
        }
    }
}
