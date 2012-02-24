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

import QtQuick 1.1
import org.kde.qtextracomponents 0.1
import org.kde.plasma.components 0.1

PathView {
    id: view
    delegate: Column {
            height: 80
            width: 80
            spacing: 15
            QIconItem { icon: model["icon"]; width: 40; height: 40; anchors.horizontalCenter: parent.horizontalCenter }
            Label { height: 10; text: label; color: parent.PathView.isCurrentItem ? "red" : "black"; anchors.horizontalCenter: parent.horizontalCenter }
        }
    path: Path {
        id: p
        startX: view.width/2; startY: 25
        PathQuad { x: view.width/2; y: 250; controlX: view.width; controlY: 100 }
        PathQuad { x: view.width/2; y: 25; controlX: 0; controlY: 100 }
    }
}