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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1
import org.kde.draganddrop 1.0

Item {
    id: listItem
    width: ListView.view.width
    height: listItemDelegate.height + listItemDelegate.anchors.margins*2

    property bool dropEnabled: false
    property bool modelChildren: hasModelChildren

    function activate() {
        if (hasModelChildren) {
            listItem.ListView.view.addBreadcrumb(listItem.ListView.view.model.modelIndex(index), display);
            listItem.ListView.view.model.rootIndex = listItem.ListView.view.model.modelIndex(index);
        } else {
            launcher.openUrl(model["url"]);
            kickoff.hidePopup();
        }
    }


    Item {
        id: listItemDelegate
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: theme.defaultFont.mSize.width/2
        }
        height: Math.max(elementIcon.height, titleElement.height + subTitleElement.height)

        QIconItem {
            id: elementIcon
            icon: decoration
            width: theme.mediumIconSize
            height: width
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
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
                        return root.showAppsByName || display.length == 0 ? subtitle : display;
                    }
                } else {
                    return display;
                }
            }
            height: paintedHeight
            anchors {
                top: elementIcon.top
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
                        return root.showAppsByName || display.length == 0 ? display : subtitle;
                    }
                } else {
                    return subtitle;
                }
            }
            opacity: listItem.ListView.isCurrentItem ? 0.6 : 0
            font.pointSize: theme.smallestFont.pointSize
            elide: Text.ElideMiddle
            height: paintedHeight
            anchors {
                left: elementIcon.right
                right: parent.right
                bottom: parent.bottom
                top: titleElement.bottom
                leftMargin: 5
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.OutQuad
                }
            }
        }

        property Item contextMenu
        Component {
            id: contextMenuComponent
            Item {
                property QtObject menu: PlasmaComponents.ContextMenu {
                    id: contextMenu
                }
                
                Component.onCompleted: {
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

                    if (packagekitSource.data["Status"] && packagekitSource.data["Status"]["available"]) {
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
                    text: i18n("Add To Favorites")
                    icon: QIcon("bookmark-new")
                    onClicked: {
                        listItem.ListView.view.favoritesModel.add(model["url"]);
                    }
                }
                PlasmaComponents.MenuItem {
                    id: removeFromFavorites
                    text: i18n("Remove From Favorites")
                    icon: QIcon("list-remove")
                    onClicked: {
                        listItem.ListView.view.favoritesModel.remove(model["url"]);
                    }
                }
                PlasmaComponents.MenuItem {
                    id: sortFavoritesAscending
                    text: i18n("Sort Alphabetically (A to Z)")
                    icon: QIcon("view-sort-ascending")
                    onClicked: {
                        listItem.ListView.view.favoritesModel.sortFavoritesAscending();
                    }
                }
                PlasmaComponents.MenuItem {
                    id: sortFavoritesDescending
                    text: i18n("Sort Alphabetically (Z to A)")
                    icon: QIcon("view-sort-descending")
                    onClicked: {
                        listItem.ListView.view.favoritesModel.sortFavoritesDescending();
                    }
                }
                PlasmaComponents.MenuItem {
                    id: clearRecentApplications
                    text: i18n("Clear Recent Applications")
                    icon: QIcon("edit-clear-history")
                    onClicked: {
                        listItem.ListView.view.recentlyUsedModel.clearRecentApplications();
                    }
                }
                PlasmaComponents.MenuItem {
                    id: clearRecentDocuments
                    text: i18n("Clear Recent Documents")
                    icon: QIcon("edit-clear-history")
                    onClicked: {
                        listItem.ListView.view.recentlyUsedModel.clearRecentDocuments();
                    }
                }
                PlasmaComponents.MenuItem {
                    id: uninstallApp
                    text: i18n("Uninstall")
                    onClicked: {
                        var service = packagekitSource.serviceForSource("Status")
                        var operation = service.operationDescription("uninstallApplication")
                        operation.Url = model["url"];
                        var job = service.startOperationCall(operation)
                    }
                }
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

                        if (!listItemDelegate.contextMenu) {
                            contextMenu = contextMenuComponent.createObject(listItem).menu
                        }
                        var mapPos = listItem.mapToItem(listItem, mouse.x, mouse.y);
                        contextMenu.open(mapPos.x,mapPos.y);
                    }
                }
            }
        }
    }
}
