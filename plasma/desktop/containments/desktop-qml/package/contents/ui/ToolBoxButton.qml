/***************************************************************************
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>                    *
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
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.qtextracomponents 0.1 as QtExtras

QtExtras.QIconItem {
    id: toolBoxButton
    width: iconSize
    height: iconSize
    icon: "plasma"
    z: toolBox.z + 1

    MouseArea {
        anchors.fill: parent
        visible: toolBox.state == "collapsed"
        onClicked: ParallelAnimation {
            ScriptAction {
                script: toolBox.state = "expanded";
            }
            PlasmaExtras.AppearAnimation {
                targetItem: toolBox
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        visible: toolBox.state == "expanded"
        onClicked: SequentialAnimation {
            PlasmaExtras.DisappearAnimation {
                targetItem: toolBox
            }
            ScriptAction {
                script: toolBox.state = "collapsed";
            }
        }
    }
}
