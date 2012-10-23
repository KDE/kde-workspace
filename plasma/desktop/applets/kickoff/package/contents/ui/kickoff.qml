/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2012  Marco Martin <mart@kde.org>

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
import org.kde.plasma.extras 0.1 as PlasmaExtras
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
        searchBar.focus = true;
    }

    Component.onCompleted: {
        searchBar.focus = true;
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

    SearchBar {
        id: searchBar
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
    }

    property Item currentView: mainStack.currentTab.decrementCurrentIndex ? mainStack.currentTab : mainStack.currentTab.item

    PlasmaComponents.TabGroup {
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

        //pages
        FavoritesView {
            id: favoritesPage
        }
        PlasmaExtras.ConditionalLoader {
            id: applicationsPage
            when: mainStack.currentTab == applicationsPage
            source: Qt.resolvedUrl("ApplicationsView.qml")
        }
        PlasmaExtras.ConditionalLoader {
            id: systemPage
            when: mainStack.currentTab == systemPage
            source: Qt.resolvedUrl("SystemView.qml")
        }
        PlasmaExtras.ConditionalLoader {
            id: recentlyUsedPage
            when: mainStack.currentTab == recentlyUsedPage
            source: Qt.resolvedUrl("RecentlyUsedView.qml")
        }
        PlasmaExtras.ConditionalLoader {
            id: leavePage
            when: mainStack.currentTab == leavePage
            source: Qt.resolvedUrl("LeaveView.qml")
        }
    }

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
            tab: favoritesPage
            iconSource: "bookmarks"
            text: i18n("Favorites")
        }
        KickoffButton {
            id: applicationButton
            tab: applicationsPage
            iconSource: "applications-other"
            text: i18n("Applications")
        }
        KickoffButton {
            id: computerButton
            tab: systemPage
            iconSource: "computer" // TODO: could also be computer-laptop
            text: i18n("Computer")
        }
        KickoffButton {
            id: usedButton
            tab: recentlyUsedPage
            iconSource: "document-open-recent"
            text: i18n("Recently Used")
        }
        KickoffButton {
            id: leaveButton
            tab: leavePage
            iconSource: "system-shutdown"
            text: i18n("Leave")
        }

        onCurrentTabChanged: root.forceActiveFocus();
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
                searchBar.text += event.text;
                searchBar.focus = true;
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
            ScriptAction {script: mainStack.push(Qt.createComponent("SearchView.qml"));}
        },
        Transition {
            from: "Search"
            to: "*"
            ScriptAction {script: mainStack.pop();}
        }
    ]
}
