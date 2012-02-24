/***************************************************************************
 *   Copyright (C) 2012 by Aleix Pol Gonzalez <aleixpol@kde.org>           *
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

import org.kde.plasma.components 0.1
import org.kde.qtextracomponents 0.1
import QtQuick 1.1

ListView {
    id: resultsView
    section.property: "runnerName"
    section.delegate: Label { anchors.right: parent.right; height: 20; text: section }
    highlight: HighlightDelegate {}
    layoutDirection: ListView.RightToLeft

    delegate: ListItem {
        height: 20
        Row {
            spacing: 15
            QIconItem { icon: model["icon"]; width: height; height: 10 }
            Label { height: 10; text: label }
            
            Repeater {
                model: actions
                ToolButton { text: modelData["name"]; iconSource: icon; onClicked: trigger(); }
            }
        }
    }
}