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
    property bool switchTabsOnHover: false
    property bool showAppsByName: false
    width: 400
    height: 400
    state: "NORMAL"

    Component.onCompleted: {
        plasmoid.addEventListener('ConfigChanged', configChanged);
    }

    function configChanged() {
        root.switchTabsOnHover = plasmoid.readConfig("SwitchTabsOnHover");
        root.showAppsByName = plasmoid.readConfig("ShowAppsByName");
    }

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
            top: {
                switch (plasmoid.location) {
                case TopEdge:
                    return undefined;
                // bottom
                default:
                    return parent.top;
                }
            }
            bottom: {
                switch (plasmoid.location) {
                case TopEdge:
                    return parent.bottom;
                // bottom
                default:
                    return undefined;
                }
            }
            right: parent.right
            left: parent.left
        }
        QIconItem {
            id: searchIcon
            icon: QIcon("system-search")
            height: 32
            width: 32
            anchors {
                top: {
                    switch (plasmoid.location) {
                    case TopEdge:
                        return undefined;
                    // bottom
                    default:
                        return parent.top;
                    }
                }
                bottom: {
                    switch (plasmoid.location) {
                    case TopEdge:
                        return undefined;
                    // bottom
                    default:
                        return parent.bottom;
                    }
                }
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
                top: {
                    switch (plasmoid.location) {
                    case TopEdge:
                        return undefined;
                    // bottom
                    default:
                        return parent.top;
                    }
                }
                bottom: {
                    switch (plasmoid.location) {
                    case TopEdge:
                        return undefined;
                    // bottom
                    default:
                        return parent.bottom;
                    }
                }
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
                top: {
                    switch (plasmoid.location) {
                    case TopEdge:
                        return undefined;
                    // bottom
                    default:
                        return searchIcon.bottom;
                    }
                }
                bottom: {
                    switch (plasmoid.location) {
                    // top
                    case TopEdge:
                        return searchIcon.top;
                    // bottom
                    default:
                        return undefined;
                    }
                }
            }
            height: lineSvg.elementSize("horizontal-line").height
        }
    }
    Loader {
        id: searchView
        anchors {
            left: parent.left
            right: parent.right
            top: {
                switch (plasmoid.location) {
                case TopEdge:
                    return parent.top;
                // bottom
                default:
                    return searchBar.bottom;
                }
            }
            bottom: {
                switch (plasmoid.location) {
                case TopEdge:
                    return searchBar.top;
                // bottom
                default:
                    return parent.bottom;
                }
            }
        }
    }

    MainView {
        id: mainView
        anchors {
            top: {
                switch (plasmoid.location) {
                case TopEdge:
                    return tabBar.bottom;
                // bottom
                default:
                    return searchBar.bottom;
                }
            }
            left: parent.left
            bottom: {
                switch (plasmoid.location) {
                case TopEdge:
                    return searchBar.top;
                // bottom
                default:
                    return tabBar.top;
                }
            }
            right: parent.right
        }
    }

    Loader {
        id: applicationsViewContainer
        anchors {
            top: {
                switch (plasmoid.location) {
                case TopEdge:
                    return tabBar.bottom;
                // bottom
                default:
                    return searchBar.bottom;
                }
            }
            left: parent.left
            right: parent.right
            bottom: {
                switch (plasmoid.location) {
                case TopEdge:
                    return searchBar.top;
                // bottom
                default:
                    return tabBar.top;
                }
            }
        }
    }

    PlasmaComponents.TabBar {
        id: tabBar
        anchors {
            left: parent.left
            right: parent.right
            bottom: {
                switch (plasmoid.location) {
                case TopEdge:
                    return undefined;
                // bottom
                default:
                    return parent.bottom;
                }
            }
            top: {
                switch (plasmoid.location) {
                case TopEdge:
                    return parent.top;
                // bottom
                default:
                    return undefined;
                }
            }
        }
        KickoffButton {
            iconSource: "bookmarks"
            text: "Favorites"
            onClicked: mainView.changeModel("favorites")
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
            onClicked: mainView.changeModel("system")
        }
        KickoffButton {
            iconSource: "document-open-recent"
            text: "Recently Used"
            onClicked: mainView.changeModel("recentlyUsed")
        }
        KickoffButton {
            text: "Leave"
            iconSource: "system-shutdown"
            onClicked: mainView.changeModel("leave")
        }
    }
    Keys.onUpPressed: {
        if (root.state == "NORMAL") {
            mainView.decrementCurrentIndex();
        } else if (root.state == "APPLICATIONS") {
            applicationsViewContainer.item.decrementCurrentIndex();
        } else if (root.state == "SEARCH") {
            searchView.item.decrementCurrentIndex();
        }
        event.accepted = true;
    }
    Keys.onDownPressed: {
        if (root.state == "NORMAL") {
            mainView.incrementCurrentIndex();
        } else if (root.state == "APPLICATIONS") {
            applicationsViewContainer.item.incrementCurrentIndex();
        } else if (root.state == "SEARCH") {
            searchView.item.incrementCurrentIndex();
        }
        event.accepted = true;
    }

    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: mainView
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
                target: mainView
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
                target: mainView
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
