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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1
import org.kde.draganddrop 1.0

Item {
    id: listItem
    width: ListView.view.width
    height: listItemDelegate.height + listItemDelegate.anchors.margins*2

    property bool dropEnabled: false
    property bool modelChildren: hasModelChildren

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
            margins: theme.defaultFont.mSize.width/2
        }
        height: Math.max(elementIcon.height, titleElement.height + subTitleElement.height)

        QIconItem {
            id: elementIcon
            icon: decoration
            width: theme.mediumIconSize
            height: width
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
        }
        PlasmaComponents.Label {
            id: titleElement
            text: {
                if (root.state == "Applications") {
                    if (hasModelChildren) {
                        return display;
                    } else {
                        // TODO: games should always show the by name
                        return root.showAppsByName || display.length == 0 ? subtitle : display;
                    }
                } else {
                    return display;
                }
            }
            height: paintedHeight
            anchors {
                top: elementIcon.top
                left: elementIcon.right
                right: parent.right
                leftMargin: 5
            }
        }
        PlasmaComponents.Label {
            id: subTitleElement
            text: {
                if (root.state == "Applications") {
                    if (hasModelChildren) {
                        return "";
                    } else {
                        return root.showAppsByName || display.length == 0 ? display : subtitle;
                    }
                } else {
                    return subtitle;
                }
            }
            opacity: listItem.ListView.isCurrentItem ? 0.6 : 0
            font.pointSize: theme.smallestFont.pointSize
            elide: Text.ElideMiddle
            height: paintedHeight
            anchors {
                left: elementIcon.right
                right: parent.right
                bottom: parent.bottom
                top: titleElement.bottom
                leftMargin: 5
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.OutQuad
                }
            }
        }


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
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onEntered: {
                    listItem.ListView.view.currentIndex = index;
                }
                onClicked: {
                    if (mouse.button == Qt.LeftButton)
                        activate();
                    else if (mouse.button == Qt.RightButton) {
                        // don't show a context menu for container
                        if (hasModelChildren) {
                            return;
                        }

                        var mapPos = listItem.mapToItem(listItem, mouse.x, mouse.y);
                        contextMenu.openAt(titleElement.text, decoration, model["url"], mapPos.x, mapPos.y);
                    }
                }
            }
        }
    }
}
