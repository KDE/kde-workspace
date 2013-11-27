/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2012  Marco Martin <mart@kde.org>
    Copyright (C) 2013  David Edmundson <davidedmundson@kde.org>

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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.private.kickoff 0.1 as Kickoff
import org.kde.qtextracomponents 2.0

PlasmaExtras.ConditionalLoader {
    id: root
    property int minimumWidth: theme.mSize(theme.defaultFont).width * 55
    property int minimumHeight: theme.mSize(theme.defaultFont).height * 40
    property string previousState
    property bool switchTabsOnHover: plasmoid.configuration.switchTabsOnHover
    property bool showAppsByName: plasmoid.configuration.showAppsByName

    when: plasmoid.expanded

    source: Component {
        Item {
            property Item currentView: mainStack.currentTab.decrementCurrentIndex ? mainStack.currentTab : mainStack.currentTab.item

            implicitWidth: root.minimumWidth
            implicitHeight: root.minimumHeight

            state: "Normal"
            focus: true

            PlasmaCore.DataSource {
                id: packagekitSource
                engine: "packagekit"
                connectedSources: ["Status"]
            }

            onFocusChanged: {
                root.focus = true;
            }

            Kickoff.Launcher {
                id: launcher
            }

            PlasmaCore.Svg {
                id: lineSvg
                imagePath: "widgets/line"
            }

            Timer {
                id: clickTimer

                property Item pendingButton

                interval: 250

                onTriggered: pendingButton.clicked()
            }

            SearchBar {
                id: searchBar

                anchors {
                    top: {
                        switch (plasmoid.location) {
                            case PlasmaCore.Types.TopEdge:
                            case PlasmaCore.Types.LeftEdge:
                            case PlasmaCore.Types.RightEdge:
                                return undefined;
                            default:
                                return footer.bottom;
                        }
                    }
                    bottom: {
                        switch (plasmoid.location) {
                            case PlasmaCore.Types.TopEdge:
                            case PlasmaCore.Types.LeftEdge:
                            case PlasmaCore.Types.RightEdge:
                                return footer.top;
                            default:
                                return undefined;
                        }
                    }
                    right: parent.right
                    left: parent.left
                }
            }

            Footer {
                id: footer

                anchors {
                    top: {
                        switch (plasmoid.location) {
                            case PlasmaCore.Types.TopEdge:
                            case PlasmaCore.Types.LeftEdge:
                            case PlasmaCore.Types.RightEdge:
                                return undefined;
                            default:
                                return parent.top;
                        }
                    }
                    bottom: {
                        switch (plasmoid.location) {
                            case PlasmaCore.Types.TopEdge:
                            case PlasmaCore.Types.LeftEdge:
                            case PlasmaCore.Types.RightEdge:
                                return parent.bottom;
                            default:
                                return undefined;
                        }
                    }
                    left: parent.left
                    right: parent.right
                }
            }

            PlasmaComponents.TabGroup {
                id: mainStack

                anchors {
                    top: {
                        switch (plasmoid.location) {
                        case PlasmaCore.Types.TopEdge:
                            return tabBar.bottom;
                        case PlasmaCore.Types.LeftEdge:
                        case PlasmaCore.Types.RightEdge:
                            return parent.top;
                        default:
                            return searchBar.bottom;
                        }
                    }
                    bottom: {
                        switch (plasmoid.location) {
                            case PlasmaCore.Types.TopEdge:
                            case PlasmaCore.Types.LeftEdge:
                            case PlasmaCore.Types.RightEdge:
                                return searchBar.top;
                            default:
                                return tabBar.top;
                        }
                    }
                    left: plasmoid.location == PlasmaCore.Types.LeftEdge ? parent.left : parent.left
                    right: plasmoid.location == PlasmaCore.Types.RightEdge ? tabBar.left : parent.right
                    leftMargin: plasmoid.location == PlasmaCore.Types.LeftEdge ? tabBar.height : undefined
                    rightMargin: plasmoid.location == PlasmaCore.Types.RightEdge ? tabBar.height : undefined
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
            } // mainStack

            Kickoff.FavoritesModel {
                id: favoritesModel
            }

            PlasmaComponents.TabBar {
                id: tabBar

                anchors {
                    top: {
                        switch (plasmoid.location) {
                            case PlasmaCore.Types.TopEdge:
                                return parent.top;
                            default:
                                return undefined;
                        }
                    }
                    bottom: {
                        switch (plasmoid.location) {
                            case PlasmaCore.Types.TopEdge:
                            case PlasmaCore.Types.LeftEdge:
                            case PlasmaCore.Types.RightEdge:
                                return undefined;
                            default:
                                return parent.bottom;
                        }
                    }
                    left: plasmoid.location == PlasmaCore.Types.RightEdge || plasmoid.location == PlasmaCore.Types.LeftEdge ? undefined : parent.left
                    right: plasmoid.location == PlasmaCore.Types.RightEdge || plasmoid.location == PlasmaCore.Types.LeftEdge ? undefined : parent.right
                }
                x: plasmoid.location == PlasmaCore.Types.LeftEdge ? height : (plasmoid.location == PlasmaCore.Types.RightEdge ? root.width : 0)
                width: plasmoid.location == PlasmaCore.Types.LeftEdge || plasmoid.location == PlasmaCore.Types.RightEdge ? root.height - searchBar.height - footer.height : undefined

                transformOrigin: Item.TopLeft
                rotation: plasmoid.location == PlasmaCore.Types.LeftEdge || plasmoid.location == PlasmaCore.Types.RightEdge ? 90 : 0
                currentTab: bookmarkButton

                onCurrentTabChanged: root.forceActiveFocus();

                KickoffButton {
                    id: bookmarkButton
                    tab: favoritesPage
                    iconSource: "bookmarks"
                    text: i18n("Favorites")
                    rotation: plasmoid.location == PlasmaCore.Types.LeftEdge || plasmoid.location == PlasmaCore.Types.RightEdge ? -90 : 0
                }
                KickoffButton {
                    id: applicationButton
                    tab: applicationsPage
                    iconSource: "applications-other"
                    text: i18n("Applications")
                    rotation: plasmoid.location == PlasmaCore.Types.LeftEdge || plasmoid.location == PlasmaCore.Types.RightEdge ? -90 : 0
                }
                KickoffButton {
                    id: computerButton
                    tab: systemPage
                    iconSource: "computer" // TODO: could also be computer-laptop
                    text: i18n("Computer")
                    rotation: plasmoid.location == PlasmaCore.Types.LeftEdge || plasmoid.location == PlasmaCore.Types.RightEdge ? -90 : 0
                }
                KickoffButton {
                    id: usedButton
                    tab: recentlyUsedPage
                    iconSource: "document-open-recent"
                    text: i18n("Recently Used")
                    rotation: plasmoid.location == PlasmaCore.Types.LeftEdge || plasmoid.location == PlasmaCore.Types.RightEdge ? -90 : 0
                }
                KickoffButton {
                    id: leaveButton
                    tab: leavePage
                    iconSource: "system-shutdown"
                    text: i18n("Leave")
                    rotation: plasmoid.location == PlasmaCore.Types.LeftEdge || plasmoid.location == PlasmaCore.Types.RightEdge ? -90 : 0
                }
            } // tabBar

            Keys.forwardTo: [tabBar.layout]

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
            ] // states

            Component.onCompleted: {
                root.focus = true;
            }
        }
    }
} // root
