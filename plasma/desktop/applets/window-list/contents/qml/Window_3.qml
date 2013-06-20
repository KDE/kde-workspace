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
import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts
import org.kde.qtextracomponents 0.1 as QtExtra

Item {
    id: fontsi
    width: parent.width
    height: fontstyle.height + padding
    clip: true
    property string text
    property int padding : 6
    property int fontSize : 8
    property alias font: fontstyle.font
    property int interval : 40
    property color textColor: "black"
        Text {
	anchors.verticalCenter: parent.verticalCenter
	id: fontstyle
	font.pointSize: fontSize
	color: textColor
	text: parent.text
        }
}
