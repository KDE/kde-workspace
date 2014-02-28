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
    property alias model: kickoffListView.model

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
        anchors.fill: parent

        ListView {
            id: kickoffListView

            anchors.fill: parent

            interactive: contentHeight > height
            boundsBehavior: Flickable.StopAtBounds
            currentIndex: -1
            highlight: PlasmaComponents.Highlight {}

            delegate: KickoffItem {}

            section {
                property: "group"
                criteria: ViewSection.FullString
                delegate: SectionDelegate {}
            }
        }
    }
}
