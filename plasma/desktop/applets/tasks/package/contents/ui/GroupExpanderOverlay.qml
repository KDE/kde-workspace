/***************************************************************************
 *   Copyright (C) 2012-2013 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 1.1

import org.kde.plasma.core 0.1 as PlasmaCore

PlasmaCore.SvgItem {
    anchors {
        bottom: parent.bottom
        horizontalCenter: iconBox.horizontalCenter
    }

    width: Math.min(theme.smallIconSize, iconBox.width)
    height: width

    svg: arrows
    elementId: elementForLocation()

    function elementForLocation()
    {
        switch (tasks.location) {
            case LeftEdge:
                return "right-arrow";
            case TopEdge:
                return "down-arrow";
            case RightEdge:
                return "left-arrow";
            case BottomEdge:
            default:
                return "up-arrow";
        }
    }
}
