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
import org.kde.qtextracomponents 0.1

Item {
    id: root
    property int minimumWidth: 290
    property int minimumHeight: 340
    property string previousState
    width: 400
    height: 400
    state: "NORMAL"

    Kickoff.Launcher {
        id: launcher
    }

    PlasmaCore.Svg {
        id: lineSvg
        imagePath: "widgets/line"
    }
    Component {
        id: kickoffDelegate
        KickoffItem {}
    }
    Item {
        id: searchBar
        height: 32 + lineSvg.elementSize("horizontal-line").height
        anchors {
            top: parent.top
            right: parent.right
            left: parent.left
        }
        QIconItem {
            id: searchIcon
            icon: QIcon("system-search")
            height: 32
            width: 32
            anchors {
                top: parent.top
                left: parent.left
            }
        }
        PlasmaComponents.TextField {
            id: searchField
            placeholderText: "Search"
            clearButtonShown: true
            anchors {
                left: searchIcon.right
                right: parent.right
                top: parent.top
                leftMargin: 5
            }
            onTextChanged: {
                if (searchView.source == "") {
                    searchView.source = "SearchView.qml";
                }
                if (root.state != "SEARCH") {
                    root.previousState = root.state;
                    root.state = "SEARCH";
                }
                if (text == "") {
                    root.state = root.previousState;
                }
            }
        }
        PlasmaCore.SvgItem {
            svg: lineSvg
            elementId: "horizontal-line"
            anchors {
                left: parent.left
                right: parent.right
                top: searchIcon.bottom
            }
            height: lineSvg.elementSize("horizontal-line").height
        }
    }
    Loader {
        id: searchView
        anchors {
            left: parent.left
            right: parent.right
            top: searchBar.bottom
            bottom: parent.bottom
        }
    }
    ListView {
        id: kickoffListView
        property variant favoritesModel
        property variant leaveModel
        property variant systemModel
        property variant recentlyUsedModel
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
        anchors {
            top: searchBar.bottom
            left: parent.left
            bottom: tabBar.top
            right: scrollBar.visible ? scrollBar.left : parent.right
        }
        clip: true
        focus: true
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
    }

    PlasmaComponents.ScrollBar {
        id: scrollBar
        flickableItem: kickoffListView
        anchors {
            right: parent.right
            top: searchBar.bottom
            bottom: tabBar.top
        }
    }

    Loader {
        id: applicationsViewContainer
        anchors {
            top: searchBar.bottom
            left: parent.left
            right: parent.right
            bottom: tabBar.top
        }
    }

    PlasmaComponents.TabBar {
        id: tabBar
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        KickoffButton {
            iconSource: "bookmarks"
            text: "Favorites"
            onClicked: kickoffListView.changeModel("favorites")
        }
        KickoffButton {
            iconSource: "applications-other"
            text: "Applications"
            onClicked: {
                if (applicationsViewContainer.source == "") {
                    applicationsViewContainer.source = "ApplicationsView.qml";
                }
                root.state = "APPLICATIONS";
            }
        }
        KickoffButton {
            iconSource: "computer" // TODO: could also be computer-laptop
            text: "Computer"
            onClicked: kickoffListView.changeModel("system")
        }
        KickoffButton {
            iconSource: "document-open-recent"
            text: "Recently Used"
            onClicked: kickoffListView.changeModel("recentlyUsed")
        }
        KickoffButton {
            text: "Leave"
            iconSource: "system-shutdown"
            onClicked: kickoffListView.changeModel("leave")
        }
    }
    Keys.onUpPressed: {
        if (root.state == "NORMAL") {
            kickoffListView.decrementCurrentIndex();
        } else if (root.state == "APPLICATIONS") {
            applicationsView.decrementCurrentIndex();
        }
        event.accepted = true;
    }
    Keys.onDownPressed: {
        if (root.state == "NORMAL") {
            kickoffListView.incrementCurrentIndex();
        } else if (root.state == "APPLICATIONS") {
            applicationsView.incrementCurrentIndex();
        }
        event.accepted = true;
    }

    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: kickoffListView
                visible: true
            }
            PropertyChanges {
                target: applicationsViewContainer
                visible: false
            }
            PropertyChanges {
                target: tabBar
                visible: true
            }
            PropertyChanges {
                target: searchView
                visible: false
            }
        },
        State {
            name: "APPLICATIONS"
            PropertyChanges {
                target: kickoffListView
                visible: false
            }
            PropertyChanges {
                target: scrollBar
                visible: false
            }
            PropertyChanges {
                target: applicationsViewContainer
                visible: true
            }
            PropertyChanges {
                target: tabBar
                visible: true
            }
            PropertyChanges {
                target: searchView
                visible: false
            }
        },
        State {
            name: "SEARCH"
            PropertyChanges {
                target: kickoffListView
                visible: false
            }
            PropertyChanges {
                target: scrollBar
                visible: false
            }
            PropertyChanges {
                target: applicationsViewContainer
                visible: false
            }
            PropertyChanges {
                target: tabBar
                visible: false
            }
            PropertyChanges {
                target: searchView
                visible: true
            }
        }
    ]
}
