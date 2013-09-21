/*
 * Copyright 2013  Bhushan Shah <bhush94@gmail.com>
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
 *
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.icon 1.0

Column {
    id: root

    PlasmaCore.IconItem {
        id: icon
        anchors.horizontalCenter: parent
        width: theme.hugeIconSize
        height: width
        source: plasmoid.configuration.iconName
    }

    PlasmaComponents.Label {
        id: label
        anchors.horizontalCenter: parent
        width: parent.width - icon.width
        wrapMode: Text.Wrap
        text: plasmoid.configuration.applicationName
    }
    
    Logic {
	id: logic
    }
    
    Connections {
	target: plasmoid
	onExternalData: {
	    print("************************************************************************************************");
	    print(data);
	    plasmoid.configuration.applicationName = logic.getName(data);
	    plasmoid.configuration.iconName = logic.getIcon(data);
	    print("************************************************************************************************");
	}
    }

}