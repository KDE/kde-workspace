/***************************************************************************
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.qtextracomponents 2.0 as QtExtraComponents

import org.kde.private.systemtray 2.0 as SystemTray
//import "plasmapackage:/code/Layout.js" as LayoutManager


Item {
    id: expandedStatusNotifier
    objectName: "expandedStatusNotifier"

    Rectangle {
        anchors.fill: parent;
        border.width: 1;
        border.color: "violet";
        color: "pink";
        visible: root.debug;
        opacity: 0.8;
    }

    Item {
        anchors {
            fill: parent
            margins: units.largeSpacing
            leftMargin: 0
        }

        PlasmaCore.IconItem {
            id: itemIcon
            width: root.baseSize * 8
            height: width
            opacity: 0.3;
            anchors {
                right: parent.right
                bottom: parent.bottom
                rightMargin: units.largeSpacing
                bottomMargin: units.largeSpacing
            }
            source: iconName != "" ? iconName : (typeof(icon) != "undefined" ? icon : "")
        }

        PlasmaComponents.Label {
            id: snTooltip

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            text: tooltipTitle
        }

        PlasmaComponents.Label {
            id: snTooltipSub

            anchors {
                top: snTooltip.bottom
                topMargin: units.largeSpacing
                left: parent.left
                right: parent.right
            }

            text: tooltipText
        }
    }
}
