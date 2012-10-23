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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.kickoff 0.1 as Kickoff
import org.kde.qtextracomponents 0.1

Item {
    id: root
    property int minimumWidth: theme.defaultFont.mSize.width * 45
    property int minimumHeight: theme.defaultFont.mSize.height * 30
    property string previousState
    property bool switchTabsOnHover: kickoff.switchTabsOnHover
    property bool showAppsByName: kickoff.showAppsByName

    width: 400
    height: 400
    state: "Normal"
    focus: true
    onFocusChanged: {
        searchField.forceActiveFocus();
    }

    Component.onCompleted: {
        searchField.forceActiveFocus();
    }

    PlasmaCore.DataSource {
        id: packagekitSource
        engine: "packagekit"
        connectedSources: ["Status"]
    }

    Kickoff.Launcher {
        id: launcher
    }

    PlasmaCore.Svg {
        id: lineSvg
        imagePath: "widgets/line"
    }

    Item {
        id: searchBar
        height: 32
        anchors {
            top: {
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
                    return undefined;
                // bottom
                default:
                    return parent.top;
                }
            }
            /*top: kickoff.location == "TopEdge" ? undefined : parent.top
            bottom: kickoff.location == "TopEdge" ? parent.bottom : undefined*/
            bottom: {
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
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
                    switch (kickoff.location) {
                    case Kickoff.Kickoff.TopEdge:
                        return undefined;
                    // bottom
                    default:
                        return parent.top;
                    }
                }
                bottom: {
                    switch (kickoff.location) {
                    case Kickoff.Kickoff.TopEdge:
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

            placeholderText: i18nc("Search field placeholder text", "Search")
            clearButtonShown: true
            anchors {
                left: searchIcon.right
                right: parent.right
                top: {
                    switch (kickoff.location) {
                    case Kickoff.Kickoff.TopEdge:
                        return undefined;
                    // bottom
                    default:
                        return parent.top;
                    }
                }
                bottom: {
                    switch (kickoff.location) {
                    case Kickoff.Kickoff.TopEdge:
                        return undefined;
                    // bottom
                    default:
                        return parent.bottom;
                    }
                }
                leftMargin: 5
            }
            onTextChanged: {
                if (root.state != "Search") {
                    root.previousState = root.state;
                    root.state = "Search";
                }
                if (text == "") {
                    root.state = root.previousState;
                    root.forceActiveFocus();
                }
            }
        }
    }

    property Item currentView: mainStack.currentPage

    PlasmaComponents.PageStack {
        id: mainStack
        anchors {
            top: {
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
                    return tabBar.bottom;
                // bottom
                default:
                    return searchBar.bottom;
                }
            }
            left: parent.left
            bottom: {
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
                    return searchBar.top;
                // bottom
                default:
                    return tabBar.top;
                }
            }
            right: parent.right
        }
        initialPage: Qt.createComponent("FavoritesView.qml")
    }

/*
    Loader {
        id: searchView
        anchors {
            left: parent.left
            right: parent.right
            top: {
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
                    return parent.top;
                // bottom
                default:
                    return searchBar.bottom;
                }
            }
            bottom: {
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
                    return searchBar.top;
                // bottom
                default:
                    return parent.bottom;
                }
            }
        }
    }

    MainView {
        id: currentView
        anchors {
            top: {
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
                    return tabBar.bottom;
                // bottom
                default:
                    return searchBar.bottom;
                }
            }
            left: parent.left
            bottom: {
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
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
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
                    return tabBar.bottom;
                // bottom
                default:
                    return searchBar.bottom;
                }
            }
            left: parent.left
            right: parent.right
            bottom: {
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
                    return searchBar.top;
                // bottom
                default:
                    return tabBar.top;
                }
            }
        }
    }
*/

    Kickoff.FavoritesModel {
        id: favoritesModel
    }

    Timer {
        id: clickTimer
        interval: 250
        property Item pendingButton
        onTriggered: pendingButton.clicked()
    }
    PlasmaComponents.TabBar {
        id: tabBar
        currentTab: bookmarkButton
        anchors {
            left: parent.left
            right: parent.right
            bottom: {
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
                    return undefined;
                // bottom
                default:
                    return parent.bottom;
                }
            }
            top: {
                switch (kickoff.location) {
                case Kickoff.Kickoff.TopEdge:
                    return parent.top;
                // bottom
                default:
                    return undefined;
                }
            }
        }
        KickoffButton {
            id: bookmarkButton
            iconSource: "bookmarks"
            text: i18n("Favorites")
            onClicked: tabBar.currentTab = bookmarkButton
        }
        KickoffButton {
            id: applicationButton
            iconSource: "applications-other"
            text: i18n("Applications")
            onClicked: tabBar.currentTab = applicationButton
        }
        KickoffButton {
            id: computerButton
            iconSource: "computer" // TODO: could also be computer-laptop
            text: i18n("Computer")
            onClicked: tabBar.currentTab = computerButton
        }
        KickoffButton {
            id: usedButton
            iconSource: "document-open-recent"
            text: i18n("Recently Used")
            onClicked: tabBar.currentTab = usedButton
        }
        KickoffButton {
            id: leaveButton
            iconSource: "system-shutdown"
            text: i18n("Leave")
            onClicked: tabBar.currentTab = leaveButton
        }

        onCurrentTabChanged: {
            root.forceActiveFocus();
            switch(tabBar.currentTab) {
                case bookmarkButton:
                    if (mainStack.currentPage.objectName !== "FavoritesView") {
                        mainStack.replace(Qt.createComponent("FavoritesView.qml"));
                    }
                    root.state = "Normal";
                    break;
                case applicationButton:
                    mainStack.replace(Qt.createComponent("ApplicationsView.qml"));
                    root.state = "Applications";
                    break;
                case computerButton:
                    mainStack.replace(Qt.createComponent("SystemView.qml"));
                    root.state = "Normal";
                    break;
                case usedButton:
                    mainStack.replace(Qt.createComponent("RecentlyUsedView.qml"));
                    root.state = "Normal";
                    break;
                case leaveButton:
                    mainStack.replace(Qt.createComponent("LeaveView.qml"));
                    root.state = "Normal";
                    break;
                default:
                    break;
            }
        }
    }

    Keys.forwardTo: [(tabBar.layout)]

    Keys.onPressed: {
        if (event.key == Qt.Key_Up) {
            if (root.state == "Normal") {
                currentView.decrementCurrentIndex();
            } else if (root.state == "Applications") {
                applicationsViewContainer.item.decrementCurrentIndex();
                applicationsViewContainer.forceActiveFocus();
            } else if (root.state == "Search") {
                searchView.item.decrementCurrentIndex();
            }
            event.accepted = true;
        }
        else if (event.key == Qt.Key_Down) {
            if (root.state == "Normal") {
                currentView.incrementCurrentIndex();
            } else if (root.state == "Applications") {
                applicationsViewContainer.item.incrementCurrentIndex();
                applicationsViewContainer.forceActiveFocus();
            } else if (root.state == "Search") {
                searchView.item.incrementCurrentIndex();
            }
            event.accepted = true;
        }
        else if (event.key == Qt.Key_Tab) {
            if (root.state == "Applications") {
                if (applicationsViewContainer.activeFocus )
                    root.forceActiveFocus();
                else
                    applicationsViewContainer.forceActiveFocus();
            }
            event.accepted = true;
        }
        else if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
            if (root.state == "Normal") {
                currentView.activateCurrentIndex();
            } else if (root.state == "Search") {
                searchView.item.activateCurrentIndex();
            }
            event.accepted = true;
        }
        else { // forward key to searchView
            if (event.text != "") {
                searchField.text += event.text;
                searchField.forceActiveFocus();
            }
            event.accepted = true;
        }
    }

    states: [
        State {
            name: "Normal"
            PropertyChanges {
                target: tabBar
                visible: true
            }
        },
        State {
            name: "Applications"
            PropertyChanges {
                target: tabBar
                visible: true
            }
        },
        State {
            name: "Search"
            PropertyChanges {
                target: tabBar
                visible: false
            }
        }
    ]
    transitions: [
        Transition {
            from: "*"
            to: "Search"
            ScriptAction {script: currentView.push(Qt.createComponent("SearchView.qml"));}
        },
        Transition {
            from: "Search"
            to: "*"
            ScriptAction {script: currentView.pop();}
        }
    ]
}
