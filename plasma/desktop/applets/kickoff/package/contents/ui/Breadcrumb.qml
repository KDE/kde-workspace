/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

PlasmaComponents.ToolButton {
    id: crumb

    property bool root: false
    property int depth: model.depth
    enabled: depth < crumbModel.count

    onClicked: {
        if (root) {
            applicationsView.model.rootIndex = 0
        } else {
            applicationsView.model.rootIndex = model.modelIndex;
        }

        while (crumbModel.count > 0 &&
            depth < crumbModel.get(crumbModel.count-1).depth) {
            crumbModel.remove(crumbModel.count-1)
        }
    }
}
