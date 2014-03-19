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
import org.kde.qtextracomponents 2.0 as QtExtra
import org.kde.milou 0.1 as Milou
import "globals.js" as Globals

QtExtra.MouseEventListener {
    id: resultDelegate
    property variant theModel: model

    width: listItem.implicitWidth
    height: listItem.implicitHeight

    hoverEnabled: true

//     onContainsMouseChanged: {
//         if (containsMouse) {
//             preview.mimetype = model.previewType;
//             preview.url = model.previewUrl;
//             urlLabel.text = model.previewLabel
//
//             dialog.delegate = resultDelegate;
//         } else {
//             dialog.prevDelegate = dialog.delegate
//             dialog.delegate = null;
//         }
//     }

    // FIXME
    // This has been done so that the current item looks selected
    // We need a proper fix in PlasmaComponents.ListItem to show the currentItem
    // as selected
    opacity: ListView.isCurrentItem ? 0.5 : 1

    PlasmaComponents.ListItem {
        id: listItem
        enabled: true

        // Try to find some signal called activated? and use that?
        onClicked: {
            listView.currentIndex = model.index
            listView.model.run(model.index);
            view.clearPreview();
        }

        QtExtra.QIconItem {
            id: typePixmap
            width: Globals.IconSize
            height: Globals.IconSize

            icon: model.decoration
            smooth: true

            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
                leftMargin: Globals.CategoryWidth
            }
        }

        PlasmaComponents.Label {
            id: displayLabel
            text: model.display
            wrapMode: Text.Wrap

            width: parent.width
            height: typePixmap.height

            elide: Text.ElideRight
            maximumLineCount: 1

            anchors {
                left: typePixmap.right
                leftMargin: 5
                verticalCenter: parent.verticalCenter
                right: parent.right
            }
        }
    }
}
