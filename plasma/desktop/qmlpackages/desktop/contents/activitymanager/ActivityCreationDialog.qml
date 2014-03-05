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
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.activities 0.1 as Activities

PlasmaComponents.CommonDialog {
    id: root

    property alias activityName: textActivityName.text

    titleText: i18n("Create a new activity")
    titleIcon: "preferences-activities"

    content: Column {
        width: parent.width

        PlasmaComponents.Label {
            text: i18n("Activity name:")

            elide: Text.ElideRight

            width: parent.width
        }

        PlasmaComponents.TextField {
            id: textActivityName

            width: parent.width

            text: "Test Activity"

        }

    }

    buttonTexts: [i18n("Create"), i18n("Dismiss")]
}
