/*
 *  Copyright 2013 David Edmundson <davidedmundson@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls

//FIXME this causes a crash in Oxygen style
//QtControls.GroupBox {
Item {
    width: childrenRect.width
    height: childrenRect.height

//FIXME enable when we're back to being a group box
//     flat: true
//     title: i18n("Appearance")

    property alias cfg_showSecondHand: showSecondHandCheckBox.checked
    property alias cfg_showTimezoneString: showTimezoneCheckBox.checked

    Column {
        QtControls.CheckBox {
            id: showSecondHandCheckBox
            text: i18n("Show seconds hand")
        }
        QtControls.CheckBox {
            id: showTimezoneCheckBox
            text: i18n("Show time zone")
        }
    }
}
