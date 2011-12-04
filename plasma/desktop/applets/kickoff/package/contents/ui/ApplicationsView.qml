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

Item {
    anchors.fill: parent
    PlasmaComponents.ButtonRow {
        id: breadcrumbsElement
        exclusive: false
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        Breadcrumb {
            id: rootBreadcrumb
            root: true
            text: "All Applications"
            enabled: false
        }
    }
    ListView {
        id: applicationsView
        function addBreadcrumb(modelIndex, title) {
            breadcrumbs.children[breadcrumbs.children.length-1].enabled = true;
            var crumb = Qt.createQmlObject('import QtQuick 1.1; Breadcrumb{}', breadcrumbs, title + "BreadcrumbSnippet");
            crumb.text = title;
            crumb.modelIndex = modelIndex;
            crumb.view = applicationsView;
            crumb.enabled = false;
        }
        property variant breadcrumbs: breadcrumbsElement
        anchors {
            top: breadcrumbsElement.bottom
            left: parent.left
            bottom: parent.bottom
            right: applicationsScrollBar.visible ? applicationsScrollBar.left : parent.right
        }
        model: VisualDataModel {
            model: Kickoff.ApplicationModel {}
            delegate: KickoffItem {
                id: kickoffItem
                PlasmaCore.SvgItem {
                    svg: arrowSvg
                    elementId: "right-arrow"
                    height: arrowSvg.elementSize("right-arrow").height
                    width: arrowSvg.elementSize("right-arrow").width
                    visible: hasModelChildren
                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }
                }
            }
        }
        highlight: PlasmaComponents.Highlight {}
        PlasmaCore.Svg {
            id: arrowSvg
            imagePath: "widgets/arrows"
        }
        clip: true
        Component.onCompleted: {
            rootBreadcrumb.modelIndex = model.rootIndex;
            rootBreadcrumb.view = applicationsView;
        }
    }
    PlasmaComponents.ScrollBar {
        id: applicationsScrollBar
        flickableItem: applicationsView
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
    }
}
