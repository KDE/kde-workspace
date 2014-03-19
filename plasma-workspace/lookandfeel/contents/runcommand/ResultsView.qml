/*
 * This file is part of the KDE Milou Project
 * Copyright (C) 2013-2014 Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.1

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
// import org.kde.qtextracomponents 2.0s as QtExtra
import org.kde.milou 0.1 as Milou

import "globals.js" as Globals

ListView {
    id: listView
    property alias queryString: resultModel.queryString

    clip: true

    // This is used to keep track if the user has pressed enter before
    // the first result has been shown, in the case the first result should
    // be run when the model is populated
    property bool runAutomatically

    model: Milou.ReverseModel {
        sourceModel: Milou.SourcesModel {
            id: resultModel
            queryLimit: 20
        }

        // Internally when the query string changes, the model is reset
        // and the results are presented
        onModelReset: {
            if (reversed) {
                listView.currentIndex = listView.count - 1
            }
            else {
                listView.currentIndex = 0
            }

            if (runAutomatically) {
                runCurrentIndex();
                runAutomatically = false
            }
        }

        reversed: plasmoid.location == PlasmaCore.BottomEdge
    }

    delegate: ResultDelegate {
        id: resultDelegate
        width: listView.width
    }

    //
    // vHanda: Ideally this should have gotten handled in the delagte's onReturnPressed
    // code, but the ListView doesn't seem forward keyboard events to the delgate when
    // it is not in activeFocus. Even manually adding Keys.forwardTo: resultDelegate
    // doesn't make any difference!
    Keys.onReturnPressed: {
        if (!currentIndex) {
            runAutomatically = true
        }
        runCurrentIndex();
    }

    function runCurrentIndex() {
        listView.model.run(currentIndex);
        clearPreview();
    }

    Keys.onTabPressed: incrementCurrentIndex()
    Keys.onBacktabPressed: decrementCurrentIndex()

    boundsBehavior: Flickable.StopAtBounds

    Component {
        id: sectionDelegate
        Item {
            width: Globals.CategoryWidth

            // The height is 0 cause we do not want the component to consume
            // any height. We put it on the left
            height: 0

            // We're creating a ListItem so that we can use its height to set
            // the items height. This is similar to the ResultDelegate code
            PlasmaComponents.ListItem {
                opacity: 0
                id: blah
                Item {
                    height: Globals.IconSize
                }
            }

            Item {
                width: parent.width
                height: blah.implicitHeight

                PlasmaComponents.Label {
                    id: sectionText
                    text: section
                    color: theme.textColor
                    opacity: 0.5

                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter

                    anchors {
                        fill: parent
                        rightMargin: Globals.CategoryRightMargin
                    }
                }
            }
        }
    }

    section.property: "type"
    section.delegate: sectionDelegate

    function loadSettings() {
        resultModel.loadSettings()
    }

    function setQueryString(string) {
        resultModel.queryString = string
//         preview.highlight = string
        runAutomatically = false
    }

    onCurrentItemChanged: {
//         showPreview();
    }

    // Tooltip
//     PlasmaCore.Dialog {
//         id: dialog
//         property Item delegate
//         property Item prevDelegate
//
//         mainItem: QtExtra.MouseEventListener {
//             hoverEnabled: true
//
//             width: childrenRect.width
//             height: childrenRect.height
//
//             onContainsMouseChanged: {
//                 if (containsMouse) {
//                     if (dialog.visible)
//                         hideTimer.stop()
//                 }
//                 else {
//                     hideTimer.start()
//                 }
//             }
//
//             Milou.Preview {
//                 id: preview
//
//                 onLoadingFinished: {
//                     if (!dialog.delegate)
//                         return
//
//                     var height = preview.height + urlLabel.height + urlLabel.anchors.topMargin
//                     var point = plasmoid.tooltipPosition(dialog.delegate, preview.width, height)
//                     dialog.x = point.x
//                     dialog.y = point.y
//
//                     // dialog.visible = true
//                     // We cannot do this because PlasmaCore Dialog is strange. If we just set visible
//                     // to true, the width and height are never updated
//                     // Therefore we give it time to update its width and height
//                     plasmaDialogIsSlowTimer.start();
//                 }
//
//                 Timer {
//                     id: plasmaDialogIsSlowTimer
//                     // Plasma::Dialog has a timer of 150 internally
//                     interval: 155
//                     repeat: false
//
//                     onTriggered: {
//                         dialog.visible = true
//                     }
//                 }
//             }
//
//             PlasmaComponents.Label {
//                 id: urlLabel
//                 anchors {
//                     top: preview.bottom
//                     topMargin: 5
//                 }
//                 width: preview.width
//                 height: 16
//                 elide: Text.ElideLeft
//                 horizontalAlignment: Text.AlignHCenter
//             }
//         }
//
//         Component.onCompleted: {
//             dialog.setAttribute(Qt.WA_X11NetWmWindowTypeToolTip, true)
//             dialog.windowFlags = Qt.Window|Qt.WindowStaysOnTopHint|Qt.X11BypassWindowManagerHint
//         }
//
//         // The delegate changes when the mouse hover starts on an item
//         onDelegateChanged: {
//             if (delegate) {
//                 showTimer.start()
//                 hideTimer.stop()
//
//                 if (prevDelegate != delegate) {
//                     dialog.visible = false
//                 }
//             }
//             else {
//                 showTimer.stop()
//                 hideTimer.start()
//             }
//         }
//     }
/*
    Timer {
        id: showTimer
        interval: 340
        repeat: false

        onTriggered: {
            preview.refresh();
        }
    }

    Timer {
        id: hideTimer
        interval: 500
        repeat: false

        onTriggered: {
            clearPreview()
        }
    }

    function clearPreview() {
        dialog.visible = false
        preview.clear()
    }

    function showPreview() {
        if (currentItem) {
            preview.mimetype = currentItem.theModel.previewType;
            preview.url = currentItem.theModel.previewUrl;
            urlLabel.text = currentItem.theModel.previewLabel

            dialog.delegate = null
            dialog.delegate = currentItem
        }
    }*/
}
