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
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.private.kickoff 0.1 as Kickoff
import org.kde.draganddrop 2.0


Item {
    anchors.fill: parent

    objectName: "FavoritesView"

    function decrementCurrentIndex() {
        kickoffListView.decrementCurrentIndex();
    }

    function incrementCurrentIndex() {
        kickoffListView.incrementCurrentIndex();
    }

    function activateCurrentIndex() {
        kickoffListView.currentItem.activate();
    }

    ContextMenu {
        id: contextMenu

        PlasmaComponents.MenuItem {
            id: actionsSeparator

            separator: true
        }
        PlasmaComponents.MenuItem {
            id: sortFavoritesAscending

            text: i18n("Sort Alphabetically (A to Z)")
            icon: "view-sort-ascending"

            onClicked: {
                favoritesModel.sortFavoritesAscending();
            }
        }
        PlasmaComponents.MenuItem {
            id: sortFavoritesDescending

            text: i18n("Sort Alphabetically (Z to A)")
            icon: "view-sort-descending"

            onClicked: {
                favoritesModel.sortFavoritesDescending();
            }
        }
    }

    PlasmaExtras.ScrollArea {
        id: scrollArea

        anchors.fill: parent

        ListView {
            id: kickoffListView

            anchors.fill: parent

            interactive: contentHeight > height
            delegate: KickoffItem {}
            highlight: PlasmaComponents.Highlight {}

            model: favoritesModel

            section {
                property: "group"
                criteria: ViewSection.FullString
                delegate: SectionDelegate {}
            }
        }
    }

    DropArea {
        anchors.fill: scrollArea

        function syncTarget(event) {
            kickoffListView.currentIndex = kickoffListView.indexAt(event.x, event.y + kickoffListView.contentY)

            if (kickoffListView.currentIndex === -1) {
                if (event.y < height/2) {
                    kickoffListView.currentIndex = 0
                } else {
                    kickoffListView.currentIndex = kickoffListView.count - 1
                }
            }
            if (event.y < kickoffListView.currentItem.y + kickoffListView.currentItem.height / 2) {
                dropTarget.y = kickoffListView.currentItem.y - kickoffListView.contentY
            } else {
                dropTarget.y = kickoffListView.currentItem.y + kickoffListView.currentItem.height - kickoffListView.contentY
            }
        }

        onDrop: {
            var row = kickoffListView.currentIndex
            if (event.y + kickoffListView.contentY < kickoffListView.currentItem.y + kickoffListView.currentItem.height / 2) {
                --row
            }
            kickoffListView.model.dropMimeData(event.mimeData.text, event.mimeData.urls, row, 0);
            dropTarget.visible = false;
        }
        onDragEnter: {
            syncTarget(event);
            dropTarget.visible = true;
        }
        onDragMove: syncTarget(event);

        onDragLeave: {
            dropTarget.visible = false;
        }

        Rectangle {
            id: dropTarget

            width: parent.width
            height: 2

            visible: false
            color: theme.highlightColor
        }
    }
}
