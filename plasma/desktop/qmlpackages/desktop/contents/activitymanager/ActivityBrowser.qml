/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.activities 0.1 as Activities

Item {
    id: root
    width: 208

    property int spacing: 2 * units.smallSpacing

    signal closeRequested()

    focus: true

    Keys.onPressed: {
        if (event.key == Qt.Key_Escape) {
            if (heading.showSearch)
                heading.showSearch = false;
            else
                root.closeRequested();

        } else if (event.key == Qt.Key_Up) {
            console.log("UP KEY");

        } else if (event.key == Qt.Key_Down) {
            console.log("DOWN KEY");

        } else if (event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
            console.log("ENTER KEY");

        } else if (event.key == Qt.Key_Tab) {
            console.log("TAB KEY");

        } else  {
            console.log("OTHER KEY");
            heading.showSearch = true;

        }

    }

    // Rectangle {
    //     anchors.fill : parent
    //     opacity      : .4
    //     color        : "white"
    // }

    Heading {
        id: heading

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right

            topMargin: units.largeSpacing
        }
    }

    Item {
        id: bottomPanel

        height: newActivityButton.height + units.largeSpacing

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        PlasmaComponents.ToolButton {
            id: newActivityButton

            text: "Create activity..."
            iconSource: "list-add"

            width: parent.width

            onClicked: {
                console.log("New activity");
                newActivityDialog.open()
            }
        }

        ActivityCreationDialog {
            id: newActivityDialog

            visualParent: newActivityButton

            anchors {
                bottom: newActivityButton.top
            }

            onButtonClicked: {
                if (index == 0) {
                    console.log("Create activity: " + activityName)
                    activityList.model.addActivity(activityName, function () {})
                }
            }

        }
    }

    PlasmaExtras.ScrollArea {
        anchors {
            top: heading.bottom
            bottom: bottomPanel.top
            left: parent.left
            right: parent.right
            topMargin: root.spacing
        }

        flickableItem: ActivityList {
            id: activityList

            filterString: heading.searchString.toLowerCase()
        }
    }
}

