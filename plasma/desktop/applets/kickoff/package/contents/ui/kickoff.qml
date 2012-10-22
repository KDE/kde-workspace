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
    state: "NORMAL"
    focus: true
    onFocusChanged: {
        searchField.forceActiveFocus();

    }

    Component.onCompleted: {
        searchField.forceActiveFocus();
    }

    function action_menuEditor() {
        plasmoid.runApplication("kmenuedit.desktop");
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
    Component {
        id: kickoffDelegate
        KickoffItem {
            dropEnabled: ListView.view.model == favoritesModel
        }
    }
    Item {
        id: searchBar
        height: 32 + lineSvg.elementSize("horizontal-line").height
        anchors {
            top: {
                switch (kickoff.location) {
                case Kickoff.TopEdge:
                    return undefined;
                // bottom
                default:
                    return parent.top;
                }
            }
            bottom: {
                switch (kickoff.location) {
                case Kickoff.TopEdge:
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
                    case Kickoff.TopEdge:
                        return undefined;
                    // bottom
                    default:
                        return parent.top;
                    }
                }
                bottom: {
                    switch (kickoff.location) {
                    case Kickoff.TopEdge:
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
                    case Kickoff.TopEdge:
                        return undefined;
                    // bottom
                    default:
                        return parent.top;
                    }
                }
                bottom: {
                    switch (kickoff.location) {
                    case Kickoff.TopEdge:
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
                    searchView.item.favoritesModel = mainView.favoritesModel;
                }
                if (root.state != "SEARCH") {
                    root.previousState = root.state;
                    root.state = "SEARCH";
                }
                if (text == "") {
                    root.state = root.previousState;
                    root.forceActiveFocus();
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
                    switch (kickoff.location) {
                    case Kickoff.TopEdge:
                        return undefined;
                    // bottom
                    default:
                        return searchIcon.bottom;
                    }
                }
                bottom: {
                    switch (kickoff.location) {
                    // top
                    case Kickoff.TopEdge:
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
                switch (kickoff.location) {
                case Kickoff.TopEdge:
                    return parent.top;
                // bottom
                default:
                    return searchBar.bottom;
                }
            }
            bottom: {
                switch (kickoff.location) {
                case Kickoff.TopEdge:
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
                switch (kickoff.location) {
                case Kickoff.TopEdge:
                    return tabBar.bottom;
                // bottom
                default:
                    return searchBar.bottom;
                }
            }
            left: parent.left
            bottom: {
                switch (kickoff.location) {
                case Kickoff.TopEdge:
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
                case Kickoff.TopEdge:
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
                case Kickoff.TopEdge:
                    return searchBar.top;
                // bottom
                default:
                    return tabBar.top;
                }
            }
        }
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
                case Kickoff.TopEdge:
                    return undefined;
                // bottom
                default:
                    return parent.bottom;
                }
            }
            top: {
                switch (kickoff.location) {
                case Kickoff.TopEdge:
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
                    mainView.changeModel("favorites");
                    break;
                case applicationButton: { //don't remove braces QTBUG-17012
                    if (applicationsViewContainer.source == "") {
                        applicationsViewContainer.source = "ApplicationsView.qml";
                    }
                    applicationsViewContainer.item.favoritesModel = mainView.favoritesModel;
                    root.state = "APPLICATIONS";
                    break;
                }
                case computerButton:
                    mainView.changeModel("system");
                    break;
                case usedButton:
                    mainView.changeModel("recentlyUsed");
                    break;
                case leaveButton:
                    mainView.changeModel("leave");
                    break;
                default:
                    break;
            }
        }
    }

    Keys.forwardTo: [(tabBar.layout)]

    Keys.onPressed: {
        if (event.key == Qt.Key_Up) {
            if (root.state == "NORMAL") {
                mainView.decrementCurrentIndex();
            } else if (root.state == "APPLICATIONS") {
                applicationsViewContainer.item.decrementCurrentIndex();
                applicationsViewContainer.forceActiveFocus();
            } else if (root.state == "SEARCH") {
                searchView.item.decrementCurrentIndex();
            }
            event.accepted = true;
        }
        else if (event.key == Qt.Key_Down) {
            if (root.state == "NORMAL") {
                mainView.incrementCurrentIndex();
            } else if (root.state == "APPLICATIONS") {
                applicationsViewContainer.item.incrementCurrentIndex();
                applicationsViewContainer.forceActiveFocus();
            } else if (root.state == "SEARCH") {
                searchView.item.incrementCurrentIndex();
            }
            event.accepted = true;
        }
        else if (event.key == Qt.Key_Tab) {
            if (root.state == "APPLICATIONS") {
                if (applicationsViewContainer.activeFocus )
                    root.forceActiveFocus();
                else
                    applicationsViewContainer.forceActiveFocus();
            }
            event.accepted = true;
        }
        else if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
            if (root.state == "NORMAL") {
                mainView.activateCurrentIndex();
            } else if (root.state == "SEARCH") {
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
