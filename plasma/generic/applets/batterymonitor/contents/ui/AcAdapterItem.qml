/*
 *   Copyright 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
 *   Copyright 2013 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
import org.kde.qtextracomponents 0.1

Item {
    id: brightnessItem
    clip: true
    width: parent.width
    height: acIcon.height + padding.margins.top + padding.margins.bottom

    property bool pluggedIn
    property bool showTime
    property string remainingString

    QIconItem {
        id: acIcon
        width: theme.iconSizes.dialog
        height: width
        anchors {
            top: parent.top
            topMargin: padding.margins.top
            left: parent.left
            leftMargin: padding.margins.left
        }

        icon: pluggedIn ? QIcon("battery-charging-low") : QIcon("battery-low")
    }

    Components.Label {
        id: acLabel
        anchors {
            top: acIcon.top
            left: acIcon.right
            leftMargin: 6
        }
        height: paintedHeight
        text: i18n("AC Adapter")
    }

    Components.Label {
        id: acStatus
        anchors {
            top: showTime ? acLabel.top : undefined
            bottom: showTime ? undefined : acIcon.bottom
            left: showTime ? acLabel.right : acIcon.right
            leftMargin: showTime ? 3 : 6
        }
        height: paintedHeight
        text: pluggedIn ? i18n("Plugged In") : i18n("Not Plugged In")
        color: "#77"+(theme.textColor.toString().substr(1))
    }

    Components.Label {
        id: acTime
        anchors {
            bottom: acIcon.bottom
            left: acIcon.right
            leftMargin: 6
        }
        height: paintedHeight
        visible: showTime
        text: pluggedIn ? i18n("Time remaining until full: %1", remainingString) : i18n("Time remaining until empty: %1", remainingString)
        color: "#77"+(theme.textColor.toString().substr(1))
    }
}

