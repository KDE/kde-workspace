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
    id: appViewContainer
    objectName: "ApplicationsView"

    function decrementCurrentIndex() {
        applicationsView.decrementCurrentIndex();
    }
    function incrementCurrentIndex() {
        applicationsView.incrementCurrentIndex();
    }

    anchors.fill: parent

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


    PlasmaComponents.ButtonRow {
        id: breadcrumbsElement
        exclusive: false
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        Breadcrumb {
            id: rootBreadcrumb
            root: true
            text: i18n("All Applications")
            depth: 0
        }
        Repeater {
            model: ListModel {
                id: crumbModel
            }
            Breadcrumb {
                root: false
                text: model.text
                enabled: true
            }
        }
    }
    PlasmaExtras.ScrollArea {
        anchors {
            top: breadcrumbsElement.bottom
            left: parent.left
            bottom: parent.bottom
            right: parent.right
        }
        ListView {
            id: applicationsView
            focus: true

            property variant breadcrumbs: breadcrumbsElement

            function addBreadcrumb(modelIndex, title) {
                crumbModel.append({"text": title, "modelIndex": modelIndex, "depth": crumbModel.count+1})
            }
            anchors.fill: parent
            model: VisualDataModel {
                id: vmodel

                model: Kickoff.ApplicationModel {}
                delegate: KickoffItem {
                    id: kickoffItem
                    PlasmaCore.SvgItem {
                        svg: arrowSvg
                        elementId: "right-arrow"
                        height: arrowSvg.elementSize("right-arrow").height
                        width: arrowSvg.elementSize("right-arrow").width
                        visible: hasModelChildren
                        anchors {
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
            highlight: PlasmaComponents.Highlight {}
            PlasmaCore.Svg {
                id: arrowSvg
                imagePath: "widgets/arrows"
            }
            Component.onCompleted: {
                rootBreadcrumb.modelIndex = model.rootIndex;
                rootBreadcrumb.view = applicationsView;
            }
            Keys.onPressed: {
                if (event.key == Qt.Key_Right) {
                    if (applicationsView.currentItem.modelChildren)
                        applicationsView.currentItem.activate();
                    event.accepted = true;
                }
                else if (event.key == Qt.Key_Left) {
                    if (breadcrumbs.children.length > 1) { // this is not the case when switching from the "Applications" to the "Favorites" tab using the "Left" key
                        breadcrumbs.children[breadcrumbs.children.length-2].selectCrumb();
                        event.accepted = true;
                    }
                }
                else if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
                    applicationsView.currentItem.activate();
                    event.accepted = true;
                }
            }
        }
    }
}
