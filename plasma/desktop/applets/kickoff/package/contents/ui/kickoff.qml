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
        root.focus = true;
    }

    Component.onCompleted: {
        root.focus = true;
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
        PlasmaExtras.ConditionalLoader {
            id: searchPage
            when: root.state == "Search"
            source: Qt.resolvedUrl("SearchView.qml")
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

    Keys.onPressed: {
        if (mainStack.currentTab == applicationsPage) {
            if (event.key != Qt.Key_Tab) {
                root.state = "Applications";
            }
        }

        switch(event.key) {
            case Qt.Key_Up: {
                currentView.decrementCurrentIndex();
                event.accepted = true;
                break;
            }
            case Qt.Key_Down: {
                currentView.incrementCurrentIndex();
                event.accepted = true;
                break;
            }
            case Qt.Key_Left: {
                if (!currentView.deactivateCurrentIndex()) {
                    // FIXME move to the previous tab immediately
                    root.state = "Normal"
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_Right: {
                currentView.activateCurrentIndex();
                event.accepted = true;
                break;
            }
            case Qt.Key_Tab: {
                root.state == "Applications" ? root.state = "Normal" : root.state = "Applications";
                event.accepted = true;
                break;
            }
            case Qt.Key_Enter:
            case Qt.Key_Return: {
                currentView.activateCurrentIndex(1);
                event.accepted = true;
                break;
            }
            default: { // forward key to searchView
                if (event.text != "") {
                    searchBar.query += event.text;
                    searchBar.focus = true;
                }
                event.accepted = true;
            }
        }
    }

    states: [
        State {
            name: "Normal"
            PropertyChanges {
                target: root
                Keys.forwardTo: [tabBar.layout]
            }
        },
        State {
            name: "Applications"
            PropertyChanges {
                target: root
                Keys.forwardTo: [root]
            }
        },
        State {
            name: "Search"
            PropertyChanges {
                target: tabBar
                visible: false
            }
            PropertyChanges {
                target: mainStack
                currentTab: searchPage
            }
        }
    ]
}
