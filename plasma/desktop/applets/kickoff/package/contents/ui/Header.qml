/*
 *    Copyright 2014  Sebastian Kügler <sebas@kde.org>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.kickoff 0.1 as Kickoff
//import org.kde.qtextracomponents 2.0

Item {
    id: footer

    height: units.gridUnit * 6

    Kickoff.KUser {
        id: kuser
    }

    Rectangle { color: "orange"; visible: root.debug; anchors.fill: parent; opacity: 0.3 }

    Image {
        id: faceIcon
        source: kuser.faceIconPath

        width: units.gridUnit * 4
        height: width

        anchors {
            top: parent.top
            left: parent.left
            topMargin: units.gridUnit
            leftMargin: units.gridUnit
        }
    }

    PlasmaExtras.Heading {
        id: nameLabel

        level: 2
        text: kuser.fullName

        anchors {
            left: faceIcon.right
            top: faceIcon.top
            right: parent.right
            leftMargin: units.gridUnit
            rightMargin: units.gridUnit
        }
    }

    PlasmaComponents.Label {
        text: i18n("%1 on %2", kuser.os, kuser.host)
        opacity: .4
        anchors {
            left: nameLabel.left
            bottom: faceIcon.bottom
            right: nameLabel.right
        }
    }


    Component.onCompleted: {
        print("KUser::fullName     " + kuser.fullName);
        print("KUser::loginName    " + kuser.loginName);
        print("KUser::faceIconPath " + kuser.faceIconPath);
        print("KUser::os           " + kuser.os);
        print("KUser::host         " + kuser.host);
    }
}
