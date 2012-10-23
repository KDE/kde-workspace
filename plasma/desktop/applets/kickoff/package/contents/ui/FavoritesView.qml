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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.kickoff 0.1 as Kickoff
import org.kde.draganddrop 1.0


PlasmaComponents.Page {
    anchors.fill: parent
    function decrementCurrentIndex() {
        kickoffListView.decrementCurrentIndex();
    }
    function incrementCurrentIndex() {
        kickoffListView.incrementCurrentIndex();
    }
    function activateCurrentIndex() {
        kickoffListView.currentItem.activate();
    }
    PlasmaExtras.ScrollArea {
        id: scrollArea
        anchors.fill: parent
        ListView {
            id: kickoffListView
            anchors.fill: parent
            interactive: contentHeight > height
            model: Kickoff.FavoritesModel {
                
            }

            delegate: KickoffItem {}

            Component.onCompleted: changeModel("favorites")

            section {
                property: "group"
                criteria: ViewSection.FullString
                delegate: SectionDelegate {}
            }
            highlight: PlasmaComponents.Highlight {
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
            visible: false
            width: parent.width
            height: 2
            color: theme.highlightColor
        }
    }
}
