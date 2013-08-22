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
import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.qtextracomponents 2.0
import org.kde.draganddrop 2.0

Item {
    id: listItem

    width: ListView.view.width
    height: listItemDelegate.height + listItemDelegate.anchors.margins*2

    property bool dropEnabled: false
    property bool modelChildren: hasModelChildren
    property bool appView: false

    function activate() {
        if (hasModelChildren) {
            listItem.ListView.view.addBreadcrumb(listItem.ListView.view.model.modelIndex(index), display);
            listItem.ListView.view.model.rootIndex = listItem.ListView.view.model.modelIndex(index);
        } else {
            launcher.openUrl(model["url"]);
            kickoff.hidePopup();
        }
    }


    Item {
        id: listItemDelegate

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 4 //FIXME theme.defaultFont.mSize.width/2
        }
        height: Math.max(elementIcon.height, titleElement.height + subTitleElement.height)

        DragArea {
            anchors.fill: parent
            supportedActions: Qt.MoveAction | Qt.LinkAction
            delegateImage: decoration

            mimeData {
                url: model["url"]
                source: parent
                text: index
            }

            MouseArea {
                id: mouseArea

                anchors.fill: parent

                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onEntered: {
                    listItem.ListView.view.currentIndex = index;
                }

                onClicked: {
                    if (mouse.button == Qt.LeftButton) {
                        if (appView) {
                            appViewScrollArea.state = "OutgoingLeft";
                        }
                        else {
                            listItem.activate();
                        }
                    } else if (mouse.button == Qt.RightButton) {
                        // don't show a context menu for container
                        if (hasModelChildren || typeof(contextMenu) === "undefined") {
                            return;
                        }

                        contextMenu.visualParent = mouseArea
                        contextMenu.openAt(titleElement.text, model, mouse.x, mouse.y);
                    }
                }
            }
        }

        QIconItem {
            id: elementIcon

            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
            width: theme.mediumIconSize
            height: width

            icon: decoration
        }
        PlasmaComponents.Label {
            id: titleElement

            anchors {
                top: elementIcon.top
                left: elementIcon.right
                right: parent.right
                leftMargin: 8
            }
            height: paintedHeight
            // TODO: games should always show the by name!
            text: {
                if (hasModelChildren) {
                    return display;
                } else {
                    plasmoid.configuration.showAppsByName || display.length == 0 ? (subtitle == undefined ? display : subtitle) :
                                                                    display
                }
            }
        }
        PlasmaComponents.Label {
            id: subTitleElement

            anchors {
                left: elementIcon.right
                right: parent.right
                bottom: parent.bottom
                top: titleElement.bottom
                leftMargin: 8
            }
            height: paintedHeight

            text: {
                if (hasModelChildren) {
                    return subtitle;
                } else {
                    plasmoid.configuration..showAppsByName || subtitle == undefined ? (display.length != 0 ? display : subtitle) : subtitle;
                }
            }
            opacity: 0.6
            font.pointSize: theme.smallestFont.pointSize
            elide: Text.ElideMiddle
        }
    } // listItemDelegate
} // listItem
