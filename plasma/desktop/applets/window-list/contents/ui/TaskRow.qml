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
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: menuItemContextMenu
    signal executeJob(string jobName)
    signal setOnDesktop(int desktop)
    property int desktop
    property bool minimized: false
    property bool maximized: false
    property bool shaded: false
    property bool alwaysOnTop: false
    property bool keptBelowOthers: false
    property bool fullScreen: false
    property variant desktopItems: []
    
    function open(x, y) {
        contextMenu.open(x, y);
    }
}
