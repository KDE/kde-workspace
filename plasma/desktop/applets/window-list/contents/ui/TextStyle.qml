
/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
import org.kde.qtextracomponents 0.1
Item {
    id: text_c
    height: text_style.height + padding
    clip: true
    property string text
    property int padding :0
    property int fontSize : 5
    property alias font: text_style.font
    property int interval : 40
    Components.Label {
        id:text_style
        text:parent.text
        anchors {
            verticalCenter: parent.verticalCenter
            left:parent.left
            bottom:parent.bottom
            right:parent.right
        }
        opacity:constrained ? 0 : 1
    }
}

