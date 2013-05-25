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
    height: Math.max(pmIcon.height, pmLabel.height + pmComment.height) + padding.margins.top + padding.margins.bottom

    property alias enabled: pmSwitch.checked

    signal enabledChanged(bool checked)

    QIconItem {
        id: pmIcon
        width: theme.iconSizes.dialog
        height: width
        anchors {
            verticalCenter: parent.verticalCenter
            topMargin: padding.margins.top
            bottomMargin: padding.margins.bottom
            left: parent.left
            leftMargin: padding.margins.left
        }
        icon: QIcon("preferences-system-power-management")
    }

    Components.Label {
        id: pmLabel
        anchors {
            top: parent.top
            topMargin: padding.margins.top
            left: pmIcon.right
            leftMargin: 6
        }
        height: paintedHeight
        text: i18n("Power Management")
    }

    Components.Label {
        id: pmComment
        anchors {
            bottom: parent.bottom
            bottomMargin: padding.margins.bottom
            left: pmIcon.right
            right: pmSwitch.left
            leftMargin: 6
            rightMargin: 6
        }
        elide: Text.ElideRight
        height: paintedHeight
        text: pmSwitch.checked ? i18n("Screen will be turned off automatically") : i18n("Screen will not be turned off automatically")
        color: "#77"+(theme.textColor.toString().substr(1))
    }

    Components.Switch {
        id: pmSwitch
        anchors {
            right: parent.right
            rightMargin: padding.margins.right
            verticalCenter: pmIcon.verticalCenter
        }
        checked: true
        onCheckedChanged: enabledChanged(checked)
    }
}

