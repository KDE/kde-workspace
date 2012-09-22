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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.kickoff 0.1 as Kickoff
import org.kde.draganddrop 1.0


Item {
    property alias favoritesModel: kickoffListView.favoritesModel

    function loadModelScript(name) {
        return Qt.createQmlObject('import QtQuick 1.0; import org.kde.plasma.kickoff 0.1 as Kickoff; Kickoff.' + name + '{}', kickoffListView, name + "Snippet");
    }
    function changeModel(identifier) {
        if (root.state != "NORMAL") {
            root.state = "NORMAL";
        }
        var newModel;
        switch (identifier) {
        case "favorites":
            newModel = kickoffListView.favoritesModel;
            if (!newModel) {
                newModel = loadModelScript("FavoritesModel");
                kickoffListView.favoritesModel = newModel;
            }
            break;
        case "system":
            newModel = kickoffListView.systemModel;
            if (!newModel) {
                newModel = loadModelScript("SystemModel");
                kickoffListView.systemModel = newModel;
            }
            break;
        case "recentlyUsed":
            newModel = kickoffListView.recentlyUsedModel;
            if (!newModel) {
                newModel = loadModelScript("RecentlyUsedModel");
                kickoffListView.recentlyUsedModel = newModel;
            }
            break;
        case "leave":
            newModel = kickoffListView.leaveModel;
            if (!newModel) {
                newModel = loadModelScript("LeaveModel");
                kickoffListView.leaveModel = newModel;
            }
            break;
        }
        kickoffListView.model = newModel;
    }
    function decrementCurrentIndex() {
        kickoffListView.decrementCurrentIndex();
    }
    function incrementCurrentIndex() {
        kickoffListView.incrementCurrentIndex();
    }
    function activateCurrentIndex() {
        kickoffListView.currentItem.activate();
    }
    ListView {
        id: kickoffListView
        interactive: contentHeight > height
        property variant favoritesModel
        property variant leaveModel
        property variant systemModel
        property variant recentlyUsedModel
        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
            right: scrollBar.visible ? scrollBar.left : parent.right
        }
        clip: true
        delegate: kickoffDelegate
        Component.onCompleted: changeModel("favorites")

        section {
            property: "group"
            criteria: ViewSection.FullString
            delegate: Item {
                id: sectionDelegate
                width: parent.width
                height: childrenRect.height
                PlasmaCore.SvgItem {
                    visible: sectionDelegate.y > 0
                    svg: lineSvg
                    elementId: "horizontal-line"
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    height: lineSvg.elementSize("horizontal-line").height
                }
                PlasmaComponents.Label {
                    y: 2
                    opacity: 0.6
                    text: section
                    horizontalAlignment: Text.AlignHCenter
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                }
            }
        }
        highlight: PlasmaComponents.Highlight {
        }
        footer: Item {
            height: {
                if (kickoffListView.contentHeight > kickoffListView.height) {
                    return 0;
                } else {
                    return kickoffListView.height - kickoffListView.contentHeight;
                }
            }
            width: parent.width
            DropArea {
                anchors.fill: parent
                onDrop: {
                    kickoffListView.model.dropMimeData(event.mimeData.text, event.mimeData.urls, kickoffListView.count-1, 0);
                }
            }
        }
    }

    PlasmaComponents.ScrollBar {
        id: scrollBar
        flickableItem: kickoffListView
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
    }
}
